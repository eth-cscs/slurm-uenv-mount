#include <string>
#include <vector>

std::vector<std::string> split(const std::string &s, const char delim,
                               const bool drop_empty = false);

bool is_sha(const std::string &str);
