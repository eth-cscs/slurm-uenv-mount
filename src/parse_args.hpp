#pragma once
#include "util/expected.hpp"
#include <string>
#include <vector>
#include <stdexcept>

namespace impl {

enum class protocol { file, https, jfrog };

struct mount_entry {
  enum protocol p;
  std::string image_path;
  std::string mount_point;
};

/// parse list of `:` separated tuples, enforce absolute paths, sort output by mountpoint
util::expected<std::vector<mount_entry>, std::runtime_error>
parse_arg(const std::string &arg);

}  // impl
