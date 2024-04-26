#include "helper.hpp"
#include <regex>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, char delim, int maxsplit) {
  if (maxsplit == 0) {
    return {s};
  }

  std::vector<std::string> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (!item.empty())
      elems.push_back(item);
    if (elems.size() == maxsplit) {
      elems.push_back(ss.str());
      break;
    }
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
