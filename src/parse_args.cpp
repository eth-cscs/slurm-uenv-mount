#include "parse_args.hpp"
#include "util/expected.hpp"
#include <algorithm>
#include <regex>
#include <set>
#include <stdexcept>

#include "config.hpp"
#include <slurm/spank.h>

#define LINUX_FPATH "[^\\0,:]+"

namespace impl {

const std::regex protocol_pattern("^(jfrog|file|https):\\/\\/"
                                  "(" LINUX_FPATH ")"
                                  "(:" LINUX_FPATH ")?",
                                  std::regex::ECMAScript);
const std::regex default_pattern("(" LINUX_FPATH ")"
                                 "(:" LINUX_FPATH ")?",
                                 std::regex::ECMAScript);

const std::map<std::string, enum protocol> protocol_from_string {
  {"file", protocol::file},
  {"https", protocol::https},
  {"jfrog", protocol::jfrog}
};

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
parse_arg(const std::string &arg) {
  std::vector<mount_entry> entries;
  std::vector<std::string> arguments = split(arg, ',');

  if(arguments.empty()) {
    return util::unexpected("No mountpoints given.");
  }

  for (auto &entry : arguments) {
    std::smatch match_pieces;
    if (std::regex_match(entry, match_pieces, protocol_pattern)) {
      // [protocol]::/[path]:[mountpoint]
      std::string protocol = match_pieces[1];
      std::string image_path = match_pieces[2];
      std::string mount_point;
      if (!match_pieces[3].str().empty()) {
        // remove `:` at the front
        mount_point = std::string(match_pieces[3]).erase(0, 1);
      } else {
        mount_point = DEFAULT_MOUNT_POINT;
      }
      entries.emplace_back(mount_entry{protocol_from_string.at(protocol),
                                       image_path, mount_point});
    } else if (std::regex_match(entry, match_pieces, default_pattern)) {
      std::string image_path = match_pieces[1];
      std::string mount_point;
      if (!match_pieces[2].str().empty()) {
        mount_point = std::string(match_pieces[2]).erase(0, 1);
      } else {
        mount_point = DEFAULT_MOUNT_POINT;
      }
      entries.emplace_back(
          mount_entry{protocol::file, image_path, mount_point});
    } else {
      // no match found
      return util::unexpected(
          "Invalid syntax for --uenv, expected format is: "
          "\"<file>[:mount-point][,<file:mount-point>]*\"");
    }
  }

  // check for relative paths
  for (const auto &entry : entries) {
    bool is_abs_path =
        entry.image_path[0] == '/' && entry.mount_point[0] == '/';
    if (!is_abs_path)
      return util::unexpected("Absolute path expected in " + entry.image_path +
                              ":" + entry.mount_point);
  }
  // sort by mountpoint
  std::sort(entries.begin(), entries.end(),
            [](const mount_entry &a, const mount_entry &b) {
              return a.mount_point < b.mount_point;
            });

  // check for duplicates
  std::set<std::string> set_mnt_points;
  std::for_each(entries.begin(), entries.end(),
                [&set_mnt_points](const auto &e) { set_mnt_points.insert(e.mount_point); });
  if(set_mnt_points.size() != entries.size()) {
    return util::unexpected("Duplicate mountpoints found.");
  }
  std::set<std::string> set_images;
  std::for_each(entries.begin(), entries.end(),
                [&set_images](const auto &e) {
                  set_images.insert(e.image_path);
                });
  if(set_images.size() != entries.size()) {
    return util::unexpected("Duplicate images found.");
  }

  return entries;
}

} // namespace impl
