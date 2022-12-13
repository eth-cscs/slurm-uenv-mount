extern "C" {
#include <slurm/spank.h>
}

#define ENV_MOUNT_FILE "UENV_MOUNT_FILE"
#define ENV_MOUNT_POINT "UENV_MOUNT_POINT"
// #define ENV_MOUNT_SKIP_PROLOGUE "SLURM_UENV_MOUNT_PROLOGUE"

namespace impl {

int do_mount(spank_t spank, const char *mount_point, const char *squashfs_file);

} // namespace impl
