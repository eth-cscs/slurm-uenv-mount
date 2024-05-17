#include <regex>
#include <string>
#include <vector>

#include "strings.hpp"

namespace util {

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

bool is_full_sha256(const std::string &str) {
  std::regex pattern("^[a-fA-F0-9]{64}$");
  std::smatch match;
  std::regex_match(str, match, pattern);
  if (!match.empty()) {
    return true;
  }
  return false;
}

bool is_id(const std::string &str) {
  std::regex pattern("^[a-fA-F0-9]{16}$");
  std::smatch match;
  std::regex_match(str, match, pattern);
  if (!match.empty()) {
    return true;
  }
  return false;
}

bool is_sha(const std::string &str) {
  if (is_id(str) || is_full_sha256(str)) {
    return true;
  }
  return false;
}

} // namespace util
