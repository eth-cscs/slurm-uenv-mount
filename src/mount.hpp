extern "C" {
#include <slurm/spank.h>
}
#include <string>

#define ENV_MOUNT_FILE "UENV_MOUNT_FILE"
#define ENV_MOUNT_POINT "UENV_MOUNT_POINT"

namespace impl {

int check_mount_file_is_valid(const std::string& mount_point, const std::string& squashfs_file);
int do_mount(spank_t spank, const std::string& mount_point, const std::string& squashfs_file);

} // namespace impl
