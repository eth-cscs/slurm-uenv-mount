#include <optional>
#include <string>
#include "util/expected.hpp"

struct uenv_desc {
  using entry_t = std::optional<std::string>;
  entry_t name{std::nullopt};
  entry_t version{std::nullopt};
  entry_t tag{std::nullopt};
  entry_t sha{std::nullopt};
};

uenv_desc parse_uenv_string(const std::string &str);

util::expected<std::string, std::string> find_repo_image(const uenv_desc &desc, const std::string &repo_path);
