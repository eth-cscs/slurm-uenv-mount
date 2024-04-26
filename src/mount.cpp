#include "mount.hpp"
#include "parse_args.hpp"
#include "util/expected.hpp"
#include <cstdlib>
#include <err.h>
#include <fcntl.h>
#include <iostream>
#include <libmount/libmount.h>
#include <linux/loop.h>
#include <optional>
#include <sched.h>
#include <slurm/slurm_errno.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <slurm/spank.h>
}

namespace impl {

util::expected<std::string, std::string> get_realpath(const std::string &path) {
  char *p = realpath(path.c_str(), nullptr);
  if (p) {
    std::string ret(p);
    free(p);
    return ret;
  } else {
    char *err = strerror(errno);
    return util::unexpected(err);
  }
}

bool is_valid_image(const std::string &squashfs_file) {
  struct stat mnt_stat;
  // Check that the input squashfs file exists.
  int sqsh_status = stat(squashfs_file.c_str(), &mnt_stat);
  if (sqsh_status) {
    slurm_spank_log("Invalid squashfs image \"%s\"", squashfs_file.c_str());
    return false;
  }
  if (!S_ISREG(mnt_stat.st_mode)) {
    slurm_spank_log("Invalid squashfs image \"%s\" is not a file",
                    squashfs_file.c_str());
    return false;
  }
  return true;
}

bool is_valid_mountpoint(const std::string &mount_point) {
  struct stat mnt_stat;
  auto mnt_status = stat(mount_point.c_str(), &mnt_stat);
  if (mnt_status) {
    slurm_spank_log("Invalid mount point \"%s\"", mount_point.c_str());
    return false;
  }
  if (!S_ISDIR(mnt_stat.st_mode)) {
    slurm_spank_log("Invalid mount point \"%s\" is not a directory",
                    mount_point.c_str());
    return false;
  }
  return true;
}

int do_mount(spank_t spank, const std::vector<mount_entry> &mount_entries) {
  if (mount_entries.size() == 0)
    return ESPANK_SUCCESS;
  if (unshare(CLONE_NEWNS) != 0) {
    slurm_spank_log("Failed to unshare the mount namespace");
    return -ESPANK_ERROR;
  }
  // make all mounts in new namespace slave mounts, changes in the original
  // namesapce won't propagate into current namespace
  if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) != 0) {
    slurm_spank_log("mount: unable to change `/` to MS_SLAVE | MS_REC");
    return -ESPANK_ERROR;
  }

  for (auto &entry : mount_entries) {
    std::string mount_point = entry.mount_point;
    std::string squashfs_file = entry.image_path;

    if (!is_valid_image(squashfs_file) || !is_valid_mountpoint(mount_point)) {
      return -ESPANK_ERROR;
    }

    auto cxt = mnt_new_context();

    if (mnt_context_disable_mtab(cxt, 1) != 0) {
      slurm_spank_log("Failed to disable mtab");
      return -ESPANK_ERROR;
    }

    if (mnt_context_set_fstype(cxt, "squashfs") != 0) {
      slurm_spank_log("Failed to set fstype to squashfs");
      return -ESPANK_ERROR;
    }

    if (mnt_context_append_options(cxt, "loop,nosuid,nodev,ro") != 0) {
      slurm_spank_log("Failed to set mount options");
      return -ESPANK_ERROR;
    }

    if (mnt_context_set_source(cxt, squashfs_file.c_str()) != 0) {
      slurm_spank_log("Failed to set source");
      return -ESPANK_ERROR;
    }

    if (mnt_context_set_target(cxt, mount_point.c_str()) != 0) {
      slurm_spank_log("Failed to set target");
      return -ESPANK_ERROR;
    }

    int rc = mnt_context_mount(cxt);
    if (rc != 0) {
      // https://ftp.ntu.edu.tw/pub/linux/utils/util-linux/v2.38/libmount-docs/libmount-Mount-context.html#mnt-context-mount
      char buf[256];
      rc = mnt_context_get_excode(cxt, rc, buf, sizeof(buf));
      slurm_spank_log("%s:%s", mnt_context_get_target(cxt), buf);
      return -ESPANK_ERROR;
    }
  }

  // export image, mountpoints to environment (for nested calls of sbatch)
  std::stringstream ss;
  for (auto &entry : mount_entries) {
    auto abs_image = get_realpath(entry.image_path);
    auto abs_mount = get_realpath(entry.mount_point);
    ss << "file://" << *abs_image << ":" << *abs_mount << ",";
  }
  spank_setenv(spank, UENV_MOUNT_LIST, ss.str().c_str(), 1);

  return ESPANK_SUCCESS;
}

} // namespace impl
