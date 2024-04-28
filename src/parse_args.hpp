#pragma once
#include "util/expected.hpp"
#include <optional>
#include <stdexcept>
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
/// mountpoint
util::expected<std::vector<mount_entry>, std::runtime_error>
parse_arg(const std::string &arg,
          std::optional<std::string> uenv_repo_path = std::nullopt,
          std::optional<std::string> uenv_arch = std::nullopt);

struct uenv_desc {
  using entry_t = std::optional<std::string>;
  entry_t name{std::nullopt};
  entry_t version{std::nullopt};
  entry_t tag{std::nullopt};
  entry_t sha{std::nullopt};
};

} // namespace impl
