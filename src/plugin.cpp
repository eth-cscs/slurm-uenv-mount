#include <cstdlib>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

#include "config.hpp"
#include <lib/filesystem.hpp>
#include <lib/mount.hpp>
#include <lib/parse_args.hpp>
#include <lib/strings.hpp>

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
struct arg_pack {
  std::optional<std::string> uenv_arg;
  std::optional<std::string> uenv_arch_arg;
};

static arg_pack args{};

/// wrapper for spank_getenv
std::optional<std::string> getenv(spank_t sp, const char *var) {
  const int len = 1024;
  char buf[len];

  if (spank_context() == spank_context_t::S_CTX_LOCAL ||
      spank_context() == spank_context_t::S_CTX_ALLOCATOR) {
    if (const auto ret = std::getenv(var); ret != nullptr) {
      return ret;
    }
    return std::nullopt;
  }

  const auto ret = spank_getenv(sp, var, buf, len);

  if (ret == ESPANK_ENV_NOEXIST) {
    return std::nullopt;
  }

  if (ret == ESPANK_SUCCESS) {
    return std::string{buf};
  }

  slurm_spank_log("getenv failed");
  throw ret;
}

static spank_option uenv_arg{
    (char *)"uenv",
    (char *)"<file>[:mount-point][,<file:mount-point>]*",
    (char *)"A comma seprated list of file and mountpoint, default mount-point "
            "is " DEFAULT_MOUNT_POINT,
    1, // requires an argument
    0, // plugin specific value to pass to the callback (unnused)
    [](int val, const char *optarg, int remote) -> int {
      slurm_verbose("uenv: val:%d optarg:%s remote:%d", val, optarg, remote);
      args.uenv_arg = optarg;
      return ESPANK_SUCCESS;
    }};

static spank_option uenv_arch{
    (char *)"uenv-uarch",
    (char *)"<uarch>",
    (char *)"The micro-architecture (uarch) to target. May be required to "
            "disambiguate uenv on systems with more than one node uarch. "
            "Available options are zen2, zen3, a100, mi200, gh200.",
    1, // requires an argument
    0, // plugin specific value to pass to the callback (unnused)
    [](int val, const char *optarg, int remote) -> int {
      slurm_verbose("uenv: val:%d optarg:%s remote:%d", val, optarg, remote);
      args.uenv_arch_arg = optarg;
      return ESPANK_SUCCESS;
    }};

/// detect if srun/sbatch has been called from an squashfs-run/squashfs-mount
std::optional<std::string> get_uenv_env(spank_t sp) {
  return getenv(sp, UENV_MOUNT_LIST);
}

/// return uenv_repo_path, default is $SCRATCH/.uenv-images or $UENV_REPO_PATH
/// it doesn't check if paths exist
std::optional<std::string> get_uenv_repo_path(spank_t sp) {
  auto path = getenv(sp, UENV_REPO_PATH_VARNAME);
  if (path) {
    return path.value();
  }

  auto scratch = getenv(sp, "SCRATCH");
  if (!scratch) {
    return std::nullopt;
  }
  return scratch.value() + "/.uenv-images";
}

int slurm_spank_init(spank_t sp, int ac [[maybe_unused]],
                     char **av [[maybe_unused]]) {

  for (auto arg : {&uenv_arg, &uenv_arch}) {
    if (auto status = spank_option_register(sp, arg)) {
      return status;
    }
  }

  return ESPANK_SUCCESS;
}

/// check if image, mountpoint is valid
int init_post_opt_remote(spank_t sp,
                         const std::vector<util::mount_entry> &mount_entries) {
  auto result = do_mount(mount_entries);
  if (!result) {
    slurm_spank_log("error mounting the requested uenv image: %s",
                    result.error().c_str());
    return -ESPANK_ERROR;
  }

  // export image, mountpoints to environment (for nested calls of sbatch)
  std::string env_var;
  for (auto &entry : mount_entries) {
    auto abs_image = *util::realpath(entry.image_path);
    auto abs_mount = *util::realpath(entry.mount_point);
    env_var += "file://" + abs_image + ":" + abs_mount + ",";
  }
  if (mount_entries.size() > 0) {
    spank_setenv(sp, UENV_MOUNT_LIST, env_var.c_str(), 1);
  }

  return ESPANK_SUCCESS;
}

/// check if image/mountpoint are valid
int init_post_opt_local_allocator(
    spank_t sp [[maybe_unused]],
    const std::vector<util::mount_entry> &mount_entries) {
  bool invalid_path = false;
  for (auto &entry : mount_entries) {
    if (!util::is_file(entry.image_path)) {
      invalid_path = true;
      slurm_error("Image does not exist: %s", entry.image_path.c_str());
    }
    if (!util::is_valid_mountpoint(entry.mount_point)) {
      invalid_path = true;
      slurm_error("Mountpoint is invalid: %s", entry.mount_point.c_str());
    }
  }

  if (invalid_path) {
    return -ESPANK_ERROR;
  }

  return ESPANK_SUCCESS;
}

int slurm_spank_init_post_opt(spank_t sp, int ac [[maybe_unused]],
                              char **av [[maybe_unused]]) {

  auto uenv_repo_path = get_uenv_repo_path(sp);
  std::vector<util::mount_entry> mount_entries;
  if (args.uenv_arg) {
    // parse --uenv argument, jfrog/oras is optional
    auto parsed_uenv_arg = util::parse_arg(args.uenv_arg.value(),
                                           uenv_repo_path, args.uenv_arch_arg);
    if (!parsed_uenv_arg) {
      slurm_error("%s", parsed_uenv_arg.error().c_str());
      return -ESPANK_ERROR;
    }
    mount_entries = *parsed_uenv_arg;
  } else {
    // check if UENV_MOUNT_LIST is set in environment
    if (auto uenv_mount_list = get_uenv_env(sp)) {
      // UENV_MOUNT_LIST is assumed to be fully processed, we don't query sqlite
      // here
      auto parsed_uenv_arg =
          util::parse_arg(*uenv_mount_list, /*uenv repo*/ std::nullopt);
      if (!parsed_uenv_arg) {
        slurm_error("%s", parsed_uenv_arg.error().c_str());
        return -ESPANK_ERROR;
      }
      mount_entries = *parsed_uenv_arg;
    }
  }

  switch (spank_context()) {
  case spank_context_t::S_CTX_REMOTE: {
    // mount images at specified locations
    return init_post_opt_remote(sp, mount_entries);
  }
  case spank_context_t::S_CTX_LOCAL:
  case spank_context_t::S_CTX_ALLOCATOR: {
    // valide that images and mountpoints exist
    return init_post_opt_local_allocator(sp, mount_entries);
  }
  default:
    break;
  }

  return ESPANK_SUCCESS;
}

} // namespace impl
