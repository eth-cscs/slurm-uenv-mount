#include <optional>
#include <string>

#include <err.h>
#include <libmount/libmount.h>
#include <linux/loop.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>

extern "C" {
#include <slurm/spank.h>
}

//
// Forward declare the implementation of the plugin callbacks.
//

namespace impl {
    int slurm_spank_init(spank_t sp, int ac, char **av);
    int slurm_spank_init_post_opt(spank_t sp, int ac, char **av);
    int slurm_spank_exit(spank_t sp, int ac, char **av);
} // namespace impl

//
// Implement the SPANK plugin C interface.
//

extern "C" {
    extern const char plugin_name [] = "uenv-mount";
    extern const char plugin_type [] = "spank";
    extern const unsigned int plugin_version = 1;

    // Called from both srun and slurmd.
    int slurm_spank_init(spank_t sp, int ac, char **av) {
        return impl::slurm_spank_init(sp, ac, av);
    }

    int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
        return impl::slurm_spank_init_post_opt(sp, ac, av);
    }

    int slurm_spank_exit(spank_t sp, int ac, char **av) {
        return impl::slurm_spank_exit(sp, ac, av);
    }
} // extern "C"

//
// Implementation
//

namespace impl {

    struct arg_pack {
        std::string mount_point = "/user-environment";
        std::optional<std::string> file;
        bool run_prologue = false;
    };

    static arg_pack args{};

    int slurm_spank_init(spank_t sp, int ac, char **av) {
        static spank_option mount_point_arg {
            (char*)"uenv-mount-point", (char*)"<path>",
            (char*)"path whence the environment is mounted: default /user-environment",
            1, // requires an argument
            0, // plugin specific value to pass to the callback (unnused)
            [](int val, const char *optarg, int remote) -> int {
                slurm_info ("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg, remote);
                if (!optarg) { // is this required if the has_arg flag == 1?
                    return ESPANK_BAD_ARG;
                }
                // todo: parse string to validate that the path exists
                // todo: parse string to validate that it is a valid and allowed path
                args.mount_point = optarg;
                return ESPANK_SUCCESS;
            }
        };

        static spank_option file_arg {
            (char*)"uenv-mount-file", (char*)"<path>",
            (char*)"the squashfs file with the image to mount:",
            1, // requires an argument
            0, // plugin specific value to pass to the callback (unnused)
            [](int val, const char *optarg, int remote) -> int {
                slurm_info ("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg, remote);
                if (!optarg) { // is this required if the has_arg flag == 1?
                    return ESPANK_BAD_ARG;
                }
                // todo: parse string to validate that the file exists
                // todo: parse string to validate that it is a valid and allowed path
                args.file = std::string{optarg};
                return ESPANK_SUCCESS;
            }
        };

        static spank_option prolog_arg {
            (char*)"uenv-skip-prologue", (char*)"none",
            (char*)"ignore any environment prologue script",
            0, // takes an argument
            0, // plugin specific value to pass to the callback (unnused)
            [](int val, const char *optarg, int remote) -> int {
                slurm_info ("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg, remote);
                if (optarg) { // is this required if the has_arg flag == 0?
                    return ESPANK_BAD_ARG;
                }
                args.run_prologue = false;
                return ESPANK_SUCCESS;
            }
        };

        for (auto arg: {&mount_point_arg, &file_arg, &prolog_arg}) {
            if (auto status = spank_option_register(sp, arg)) {
                return status;
            }
        }

        return ESPANK_SUCCESS;
    }

    int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
        // skip if no mount requested
        if (!args.file) {
            // TODO: log that no action was taken 
            return ESPANK_SUCCESS;
        }

        const char* mount_point = args.mount_point.c_str();
        const char* squashfs_file = args.file->c_str();

        // Check that the mount point exists.
        struct stat mnt_stat;
        auto mnt_status = stat(mount_point, &mnt_stat);
        if (mnt_status) {
            slurm_spank_log("Invalid mount point \"%s\"", mount_point);
            return ESPANK_ERROR;
        }
        if (!S_ISDIR(mnt_stat.st_mode)) {
            slurm_spank_log("Invalid mount point \"%s\" is not a directory", mount_point);
            return ESPANK_ERROR;
        }

        // Check that the input squashfs file exists.
        int sqsh_status = stat(squashfs_file, &mnt_stat);
        if (sqsh_status) {
            slurm_spank_log("Invalid squashfs image \"%s\"", squashfs_file);
            return ESPANK_ERROR;
        }
        if (!S_ISREG(mnt_stat.st_mode)) {
            slurm_spank_log("Invalid squashfs image \"%s\" is not a file", squashfs_file);
            return ESPANK_ERROR;
        }

        // TODO: do we create a new namespace when mounting directly?
        // It may be required for the "mount" MS_SLAVE|MS_REC below
        // WARNING: this might be dangerous without knowing how slurm forks and manages processes.
        if (unshare(CLONE_NEWNS) != 0) {
            slurm_spank_log("Failed to unshare the mount namespace");
            return ESPANK_ERROR;
        }

        if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) != 0) {
            slurm_spank_log("unable to mount \"%s\" image at mount pint \"%s\"", squashfs_file, mount_point);
            return ESPANK_ERROR;
        }

        auto cxt = mnt_new_context();

        if (mnt_context_disable_mtab(cxt, 1) != 0) {
            slurm_spank_log("Failed to disable mtab");
            return ESPANK_ERROR;
        }

        if (mnt_context_set_fstype(cxt, "squashfs") != 0) {
            slurm_spank_log("Failed to set fstype to squashfs");
            return ESPANK_ERROR;
        }

        if (mnt_context_append_options(cxt, "loop,nosuid,nodev,ro") != 0) {
            slurm_spank_log("Failed to set mount options");
            return ESPANK_ERROR;
        }

        if (mnt_context_set_source(cxt, squashfs_file) != 0) {
            slurm_spank_log("Failed to set source");
            return ESPANK_ERROR;
        }

        if (mnt_context_set_target(cxt, mount_point) != 0) {
            slurm_spank_log("Failed to set target");
            return ESPANK_ERROR;
        }

        if (mnt_context_mount(cxt) != 0) {
            slurm_spank_log("Failed to mount");
            return ESPANK_ERROR;
        }

        return ESPANK_SUCCESS;
    }

    int slurm_spank_exit(spank_t sp, int ac, char **av) {
        // no need to unmount - it is performed automatically when the process is killed
        return ESPANK_SUCCESS;
    }

} // namespace impl
