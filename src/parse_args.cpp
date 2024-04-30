#include "parse_args.hpp"
#include "sqlite/sqlite.hpp"
#include "util/expected.hpp"
#include "util/helper.hpp"
#include <algorithm>
#include <optional>
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

/*
 return dictionary{"name", "version", "tag", "sha" } from a uenv description
 string

 prgenv_gnu              ->("prgenv_gnu", None, None, None)
 prgenv_gnu/23.11        ->("prgenv_gnu", "23.11", None, None)
 prgenv_gnu/23.11:latest ->("prgenv_gnu", "23.11", "latest", None)
 prgenv_gnu:v2           ->("prgenv_gnu", None, "v2", None)
 3313739553fe6553        ->(None, None, None, "3313739553fe6553")
 */
uenv_desc parse_uenv_string(const std::string &entry) {
  uenv_desc res;

  if (is_sha(entry)) {
    res.sha = entry;
    return res;
  }

  std::smatch match;
  std::regex_match(entry, match, repo_pattern);

  auto name = match[1];
  auto version = match[2];
  auto tag = match[3];

  if (name.matched) {
    res.name = name.str();
  }

  if (version.matched) {
    res.version = std::string(version).erase(0, 1);
  }

  if (tag.matched) {
    res.tag = std::string(tag).erase(0, 1);
  }

  return res;
}

uenv_desc to_desc(SQLiteStatement &stmt) {
  uenv_desc desc;
  desc.name = stmt.getColumn(stmt.getColumnIndex("name"));
  desc.sha = stmt.getColumn(stmt.getColumnIndex("sha256"));
  desc.tag = stmt.getColumn(stmt.getColumnIndex("tag"));
  desc.version = stmt.getColumn(stmt.getColumnIndex("version"));
  return desc;
}

struct cmp {
  bool operator()(const uenv_desc &d1, const uenv_desc &d2) const {
    return d1.sha < d2.sha;
  }
};

util::expected<std::string, std::string>
find_repo_image(const uenv_desc &desc, const std::string &repo_path,
                std::optional<std::string> uenv_arch) {
  std::string dbpath = repo_path + "/index.db";
  // check if dbpath exists.
  if (!is_file(dbpath)) {
    return util::unexpected("Can't open uenv repo. " + dbpath +
                            " is not a file.");
  }
  SQLiteDB db(dbpath, sqlite_open::readonly);

  // get all results
  std::set<uenv_desc, cmp> shas;
  if (desc.sha) {
    if (desc.sha.value().size() < 64) {
      SQLiteStatement query(db, "SELECT * FROM records WHERE id = " +
                                    desc.sha.value());
    } else {
      SQLiteStatement query(db, "SELECT * FROM records WHERE sha256 = " +
                                    desc.sha.value());
    }
  } else {
    std::string query_str = "SELECT * FROM records WHERE ";
    std::vector<std::string> filter;
    if (uenv_arch) {
      filter.push_back("uarch");
    }
    if (desc.name) {
      filter.push_back("name");
    }
    if (desc.version) {
      filter.push_back("version");
    }
    if (desc.tag) {
      filter.push_back("tag");
    }
    for (size_t i = 0; i < filter.size(); ++i) {
      if (i > 0) {
        query_str += " AND ";
      }
      query_str += filter[i] + " = " + ":" + filter[i];
    }
    SQLiteStatement query(db, query_str);
    if (uenv_arch.has_value()) {
      query.bind(":uarch", uenv_arch.value());
    }
    if (desc.name) {
      query.bind(":name", desc.name.value());
    }
    if (desc.version) {
      query.bind(":version", desc.version.value());
    }
    if (desc.tag) {
      query.bind(":tag", desc.tag.value());
    }
    while (query.execute()) {
      shas.insert(to_desc(query));
    }
    if (shas.size() > 1) {
      std::stringstream ss;
      ss << "more than one uenv matches.\n";
      for (auto &d : shas) {
        ss << d.name.value() << "/" << d.version.value() << ":" << d.tag.value()
           << "\t" << d.sha.value() << "\n";
      }
      return util::unexpected(ss.str());
    }
    if (shas.empty()) {
      return util::unexpected(
          "No images found. Run `uenv image ls` to list available images.");
    }
  }

  return repo_path + "/images/" + shas.begin()->sha.value() + "/store.squashfs";
}

util::expected<std::vector<mount_entry>, std::runtime_error>
parse_arg(const std::string &arg, std::optional<std::string> uenv_repo_path,
          std::optional<std::string> uenv_arch) {
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
      if (!uenv_repo_path) {
        return util::unexpected("Attempting to open from uenv repository. But "
                                "either $" UENV_REPO_PATH_VARNAME
                                " or $SCRATCH is not set.");
      }
      auto image_path =
          find_repo_image(desc, uenv_repo_path.value(), uenv_arch);
      if (!image_path.has_value()) {
        return util::unexpected(image_path.error());
      }
      mount_entries.emplace_back(
          mount_entry{image_path.value(), get_mount_point(match[4])});
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
