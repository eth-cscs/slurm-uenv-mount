// #include <cstdio>
// #include <cstdlib>
// #include <stdio.h>

#include <optional>
#include <slurm/slurm_errno.h>
#include <string>

// // root version
#include "mount.hpp"

extern "C" {
#include <slurm/spank.h>
}

//
// Forward declare the implementation of the plugin callbacks.
//

namespace impl {
int slurm_spank_init(spank_t sp, int ac, char **av);
int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av);
} // namespace impl

//
// Implement the SPANK plugin C interface.
//

extern "C" {

extern const char plugin_name[] = "sqfs-spank-mount";
extern const char plugin_type[] = "spank";
#ifdef SLURM_VERSION_NUMBER
extern const unsigned int plugin_version = SLURM_VERSION_NUMBER;
#endif
extern const unsigned int spank_plugin_version = 1;

// Called from both srun and slurmd.
int slurm_spank_init(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_init(sp, ac, av);
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_task_init_privileged(sp, ac, av);
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
  static spank_option mount_point_arg{
      (char *)"uenv-mount-point",
      (char *)"<path>",
      (char *)"path whence the environment is mounted: default "
              "/user-environment",
      1, // requires an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg,
                   remote);
        if (!optarg) { // is this required if the has_arg flag == 1?
          return ESPANK_BAD_ARG;
        }
        // todo: parse string to validate that the path exists
        // todo: parse string to validate that it is a valid and allowed path
        args.mount_point = optarg;
        return ESPANK_SUCCESS;
      }};

  static spank_option file_arg{
      (char *)"uenv-mount-file",
      (char *)"<path>",
      (char *)"the squashfs file with the image to mount:",
      1, // requires an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg, remote);
        if (!optarg) { // is this required if the has_arg flag == 1?
          return ESPANK_BAD_ARG;
        }
        // check that file exists happens in do_mount
        args.file = std::string{optarg};
        return ESPANK_SUCCESS;
      }};

  static spank_option prolog_arg{
      (char *)"uenv-skip-prologue",
      (char *)"none",
      (char *)"ignore any environment prologue script",
      0, // takes an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg,
                   remote);
        if (optarg) { // is this required if the has_arg flag == 0?
          return ESPANK_BAD_ARG;
        }
        args.run_prologue = false;
        return ESPANK_SUCCESS;
      }};

  for (auto arg : {&mount_point_arg, &file_arg, &prolog_arg}) {
    if (auto status = spank_option_register(sp, arg)) {
      return status;
    }
  }

  return ESPANK_SUCCESS;
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av) {
  char env_mount_file[256];
  if (args.file) {
    return do_mount(sp, args.mount_point.c_str(), args.file->c_str());
  } else if (spank_getenv(sp, ENV_MOUNT_FILE, env_mount_file, 256) == ESPANK_SUCCESS) {
    // --uenv-mount-file flag missing, but an ENV_MOUNT_FILE is set in env
    return do_mount(sp, args.mount_point.c_str(), env_mount_file);
  }

  return ESPANK_SUCCESS;
}

} // namespace impl
