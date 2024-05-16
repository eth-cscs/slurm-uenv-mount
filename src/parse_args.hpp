#pragma once
#include "util/expected.hpp"
#include <optional>
#include <string>
#include <vector>

namespace impl {

enum class protocol { file, https, jfrog };

struct mount_entry {
  // enum protocol p;
  std::string image_path;
  std::string mount_point;
};

/// parse list of `:` separated tuples, enforce absolute paths, sort output by
/// mountpoint. The error state is an error string.
util::expected<std::vector<mount_entry>, std::string>
parse_arg(const std::string &arg,
          std::optional<std::string> uenv_repo_path = std::nullopt,
          std::optional<std::string> uenv_arch = std::nullopt);

struct uenv_desc {
  using entry_t = std::optional<std::string>;
  entry_t name;
  entry_t version;
  entry_t tag;
  entry_t sha;
};

} // namespace impl
