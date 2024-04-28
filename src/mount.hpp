extern "C" {
#include <slurm/spank.h>
}
#include <string>
#include "parse_args.hpp"

#define UENV_MOUNT_LIST "UENV_MOUNT_LIST"

namespace impl {

/// check if mountpoint is an existent directory
bool is_valid_mountpoint(const std::string &mount_point);

/// mount images and export env variable UENV_MOUNT_LIST
int do_mount(spank_t spank, const std::vector<mount_entry>& mount_entries);

} // namespace impl
