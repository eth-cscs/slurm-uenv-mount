#include "helper.hpp"
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <sys/stat.h>

extern "C" {
#include <slurm/spank.h>
}

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

bool is_file(const std::string &fname) {
  struct stat mnt_stat;
  // Check that the input squashfs file exists.
  int sqsh_status = stat(fname.c_str(), &mnt_stat);
  if (sqsh_status) {
    slurm_spank_log("Path does not exist \"%s\"", fname.c_str());
    return false;
  }
  if (!S_ISREG(mnt_stat.st_mode)) {
    slurm_spank_log("\"%s\" is not a file", fname.c_str());
    return false;
  }
  return true;
}
