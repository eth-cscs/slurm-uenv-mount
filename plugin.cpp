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

int slurm_spank_init(spank_t sp, int ac, char **av) {

  for (auto arg : {&mount_point_arg, &file_arg, &prolog_arg}) {
    if (auto status = spank_option_register(sp, arg)) {
      return status;
    }
  }
  return ESPANK_SUCCESS;
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av) {
  char env_mount_file[256];
  char env_mount_point[256];

  // indicates if slurm has been called in active squashfs-mount session.
  bool called_from_uenv{false};
  bool mount_point_override{false};
  if ((spank_getenv(sp, ENV_MOUNT_FILE, env_mount_file, 256) == ESPANK_SUCCESS) &&
      (spank_getenv(sp, ENV_MOUNT_POINT, env_mount_point, 256) == ESPANK_SUCCESS)) {
    called_from_uenv = true;
    mount_point_override = true;
    // check if mountpoint has been override by `--uenv-mount-point` flag
    if (spank_option_getopt(sp, &mount_point_arg, av) == ESPANK_SUCCESS) {
      mount_point_override = true;
    }
  }

  if (called_from_uenv) {
    std::string file = args.file ? *args.file : env_mount_file;
    std::string path = mount_point_override ? args.mount_point : env_mount_point;
    return do_mount(sp, path.c_str(), file.c_str());
  }

  // not called form an active seesion, do nothing if no sqfs file is given.
  if (args.file) {
      return do_mount(sp, args.mount_point.c_str(), args.file->c_str());
  }

  return ESPANK_SUCCESS;
}

} // namespace impl
