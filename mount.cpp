#include "mount.hpp"
#include <err.h>
#include <fcntl.h>
#include <libmount/libmount.h>
#include <linux/loop.h>
#include <optional>
#include <sched.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern "C" {
#include <slurm/spank.h>
}

namespace impl {

int do_mount(const char *mount_point, const char *squashfs_file) {
  // skip if no mount requested
  if (!squashfs_file) {
    // TODO: log that no action was taken
    slurm_spank_log("no file given to mount!");
    return ESPANK_SUCCESS;
  }

  // Check that the mount point exists.
  struct stat mnt_stat;
  auto mnt_status = stat(mount_point, &mnt_stat);
  if (mnt_status) {
    slurm_spank_log("Invalid mount point \"%s\"", mount_point);
    return ESPANK_ERROR;
  }
  if (!S_ISDIR(mnt_stat.st_mode)) {
    slurm_spank_log("Invalid mount point \"%s\" is not a directory",
                    mount_point);
    return ESPANK_ERROR;
  }

  // Check that the input squashfs file exists.
  int sqsh_status = stat(squashfs_file, &mnt_stat);
  if (sqsh_status) {
    slurm_spank_log("Invalid squashfs image \"%s\"", squashfs_file);
    return ESPANK_ERROR;
  }
  if (!S_ISREG(mnt_stat.st_mode)) {
    slurm_spank_log("Invalid squashfs image \"%s\" is not a file",
                    squashfs_file);
    return ESPANK_ERROR;
  }

  // TODO: do we create a new namespace when mounting directly?
  // It may be required for the "mount" MS_SLAVE|MS_REC below
  // WARNING: this might be dangerous without knowing how slurm forks and
  // manages processes.
  if (unshare(CLONE_NEWNS) != 0) {
    slurm_spank_log("Failed to unshare the mount namespace");
    return ESPANK_ERROR;
  }
  if (mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) != 0) {
    slurm_spank_log("unable to mount \"%s\" image at mount pint \"%s\"",
                    squashfs_file, mount_point);
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
    // https://ftp.ntu.edu.tw/pub/linux/utils/util-linux/v2.38/libmount-docs/libmount-Mount-context.html#mnt-context-mount
    char buf[256];
    rc = mnt_context_get_excode(cxt, rc, buf, sizeof(buf));
    slurm_spank_log("%s:%s", mnt_context_get_target(cxt), buf);
    return ESPANK_ERROR;
  }

  return ESPANK_SUCCESS;
}

} // namespace impl
