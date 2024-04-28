#include "parse_args.hpp"
#include "datastore.hpp"
#include "util/expected.hpp"
#include <algorithm>
#include <regex>
#include <set>
#include <stdexcept>

#include "config.hpp"
#include <slurm/spank.h>

// abs path
#define LINUX_ABS_FPATH "/[^\\0,:]+"
#define JFROG_IMAGE "[^\\0,:/]+"
namespace impl {

const std::regex default_pattern("(" LINUX_ABS_FPATH ")"
                                 "(:" LINUX_ABS_FPATH ")?",
                                 std::regex::ECMAScript);
// match <image-name>/?<version>:?<tag>:?<abs-mount-path>
const std::regex repo_pattern("(" JFROG_IMAGE ")"
                              "(/[0-9.]+)?"
                              "(:[a-zA-Z0-9]+)?"
                              "(:" LINUX_ABS_FPATH ")?",
                              std::regex::ECMAScript);

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
  }
  return elems;
}

util::expected<std::vector<mount_entry>, std::runtime_error>
parse_arg(const std::string &arg, const std::string &uenv_repo_path) {
  std::vector<std::string> arguments = split(arg, ',');

  if (arguments.empty()) {
    return util::unexpected("No mountpoints given.");
  }

  auto get_mount_point = [](std::ssub_match sub_match) -> std::string {
    if (sub_match.matched) {
      return std::string(sub_match).erase(0, 1);
    }
    return std::string{DEFAULT_MOUNT_POINT};
  };

  std::vector<mount_entry> mount_entries;
  for (auto &entry : arguments) {
    std::smatch match;
    if (std::regex_match(entry, match, default_pattern)) {
      std::string image_path = match[1];
      std::string mount_point = get_mount_point(match[2]);
      mount_entries.emplace_back(mount_entry{image_path, mount_point});
    } else if (std::regex_match(entry, match, repo_pattern)) {
      uenv_desc desc = parse_uenv_string(entry);
      auto res = find_repo_image(desc, uenv_repo_path);
      std::string mount_point = get_mount_point(match[4]);
      if (!res.has_value()) {
        return util::unexpected(res.error());
      }
      mount_entries.emplace_back(mount_entry{res.value(), mount_point});
    } else {
      // no match found
      return util::unexpected(
          "Invalid syntax for --uenv, expected format is: "
          "\"<image>[:mount-point][,<image:mount-point>]*\""
          "\n where <image> is either an absolute path or an image. Run `uenv "
          "image ls` to see a list of available images.\n"
          "mount-point must be an absolute path.");
    }
  }

  // check for relative paths
  for (const auto &entry : mount_entries) {
    bool is_abs_path =
        entry.image_path[0] == '/' && entry.mount_point[0] == '/';
    if (!is_abs_path)
      return util::unexpected("Absolute path expected in " + entry.image_path +
                              ":" + entry.mount_point);
  }
  // sort by mountpoint
  std::sort(mount_entries.begin(), mount_entries.end(),
            [](const mount_entry &a, const mount_entry &b) {
              return a.mount_point < b.mount_point;
            });

  // check for duplicates
  std::set<std::string> set_mnt_points;
  std::for_each(mount_entries.begin(), mount_entries.end(),
                [&set_mnt_points](const auto &e) {
                  set_mnt_points.insert(e.mount_point);
                });
  if (set_mnt_points.size() != mount_entries.size()) {
    return util::unexpected("Duplicate mountpoints found.");
  }
  std::set<std::string> set_images;
  std::for_each(
      mount_entries.begin(), mount_entries.end(),
      [&set_images](const auto &e) { set_images.insert(e.image_path); });
  if (set_images.size() != mount_entries.size()) {
    return util::unexpected("Duplicate images found.");
  }

  return mount_entries;
}

} // namespace impl
