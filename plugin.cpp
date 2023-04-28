#include <cstdlib>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unistd.h>

#include "mount.hpp"

extern "C" {
#include <slurm/slurm_errno.h>
#include <slurm/spank.h>
}

//
// Forward declare the implementation of the plugin callbacks.
//

namespace impl {
int slurm_spank_init(spank_t sp, int ac, char **av);
int slurm_spank_init_post_opt(spank_t sp, int ac, char **av);

} // namespace impl

//
// Implement the SPANK plugin C interface.
//

extern "C" {

extern const char plugin_name[] = "uenv-mount";
extern const char plugin_type[] = "spank";
#ifdef SLURM_VERSION_NUMBER
extern const unsigned int plugin_version = SLURM_VERSION_NUMBER;
#endif
extern const unsigned int spank_plugin_version = 1;

// Called from both srun and slurmd.
int slurm_spank_init(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_init(sp, ac, av);
}

int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_init_post_opt(sp, ac, av);
}

} // extern "C"

//
// Implementation
//
namespace impl {
enum class parse_error { invalid_path };

#define DEFAULT_MOUNT_POINT "/user-environment"
struct arg_pack {
  std::string mount_point = DEFAULT_MOUNT_POINT;
  bool mount_flag_present = false;
  std::optional<std::string> file;
  bool run_prologue = false;
};

static arg_pack args{};

/// wrapper for spank_getenv
std::optional<std::string> getenv(spank_t sp, const char *var) {
  const int len = 1024;
  char buf[len];
  spank_err_t ret = spank_getenv(sp, var, buf, len);

  if (ret == ESPANK_ENV_NOEXIST) {
    return std::nullopt;
  }

  if (ret == ESPANK_SUCCESS) {
    return std::string{buf};
  }

  throw ret;
}

static spank_option mount_point_arg{
    (char *)"uenv-mount",
    (char *)"<path>",
    (char *)"path whence the environment is mounted: "
            "default " DEFAULT_MOUNT_POINT,
    1, // requires an argument
    0, // plugin specific value to pass to the callback (unnused)
    [](int val, const char *optarg, int remote) -> int {
      slurm_verbose("uenv-mount: val:%d optarg:%s remote:%d", val, optarg,
                    remote);
      // todo: parse string to validate that the path exists
      // todo: parse string to validate that it is a valid and allowed path
      args.mount_point = optarg;
      args.mount_flag_present = true;
      return ESPANK_SUCCESS;
    }};

static spank_option file_arg{
    (char *)"uenv-file",
    (char *)"<path>",
    (char *)"the squashfs file with the image to mount:",
    1, // requires an argument
    0, // plugin specific value to pass to the callback (unnused)
    [](int val, const char *optarg, int remote) -> int {
      slurm_verbose("uenv-mount: val:%d optarg:%s remote:%d", val, optarg,
                    remote);
      // check that file exists happens in do_mount
      args.file = std::string{optarg};
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
      slurm_verbose("uenv-mount: val:%d optarg:%s remote:%d", val, optarg,
                    remote);
      args.run_prologue = false;
      return ESPANK_SUCCESS;
    }};

/// detect if srun/sbatch has been called from an squashfs-run/squashfs-mount
std::tuple<std::optional<std::string>, std::optional<std::string>>
get_squashfs_run_env(spank_t sp) {
  auto env_sqfs_file = getenv(sp, ENV_MOUNT_FILE);
  auto env_mount_point = getenv(sp, ENV_MOUNT_POINT);
  return std::make_tuple(env_sqfs_file, env_mount_point);
}

int slurm_spank_init(spank_t sp, int ac, char **av) {

  for (auto arg : {&mount_point_arg, &file_arg, &prolog_arg}) {
    if (auto status = spank_option_register(sp, arg)) {
      return status;
    }
  }

  return ESPANK_SUCCESS;
}

/// check if image, mountpoint is valid
int init_post_opt_remote(spank_t sp) {
  if (args.file) {
    return do_mount(sp, args.mount_point, *args.file);
  }
  // check if sbatch/srun/salloc was called inside squashfs-run tty
  std::optional<std::string> env_file, env_mount_point;
  try {
    std::tie(env_file, env_mount_point) = get_squashfs_run_env(sp);
  } catch (spank_err_t err) {
    slurm_error("%s", spank_strerror(err));
    return err;
  }
  if (env_file && env_mount_point) {
    return do_mount(sp, *env_mount_point, *env_file);
  }
  return ESPANK_SUCCESS;
}

/// check if image/mountpoint are valid
int init_post_opt_local_allocator(spank_t sp) {
  if (args.file) {
    return check_mount_file_is_valid(args.mount_point, *args.file);
  }
  // check if sbatch/srun/salloc was called inside squashfs-run tty
  std::optional<std::string> env_file, env_mount_point;
  try {
    std::tie(env_file, env_mount_point) = get_squashfs_run_env(sp);
  } catch (spank_err_t err) {
    slurm_error("%s", spank_strerror(err));
    return err;
  }
  if (env_file && env_mount_point) {
    return check_mount_file_is_valid(*env_mount_point, *env_file);
  }

  return ESPANK_SUCCESS;
}

int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {

  if (!args.file && args.mount_flag_present) {
    slurm_error(
        "--uenv-mount is only allowed to be used together with --uenv-file.");
    return -ESPANK_ERROR;
  }

  switch (spank_context()) {
    case spank_context_t::S_CTX_REMOTE: {
      return init_post_opt_remote(sp);
    }
    case spank_context_t::S_CTX_LOCAL:
    case spank_context_t::S_CTX_ALLOCATOR: {
      return init_post_opt_local_allocator(sp);
    }
    default:
      break;
  }

  return ESPANK_SUCCESS;
}

} // namespace impl
