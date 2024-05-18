#include <catch_amalgamated.hpp>
#include <iostream>

#include <lib/database.hpp>
#include <lib/parse_args.hpp>

// forward declare internal implementation functions for testing

namespace util {
db::uenv_desc parse_uenv_string(const std::string &entry);
}

bool operator==(const db::uenv_desc &lhs, const db::uenv_desc &rhs) {
  return (lhs.name == rhs.name) && (lhs.sha == rhs.sha) &&
         (lhs.version == rhs.version) && (lhs.tag == rhs.tag);
}

// we don't test for invalid descriptions because the calling code is expected
// to have already used a regular expression to valideate that the name is
// valid.
TEST_CASE("parse valid uenv names", "[parse_args]") {
  auto make_desc = [](const char *n, const char *v, const char *t,
                      const char *s) -> db::uenv_desc {
    db::uenv_desc desc;
    if (n)
      desc.name = n;
    if (v)
      desc.version = v;
    if (t)
      desc.tag = t;
    if (s)
      desc.sha = s;
    return desc;
  };

  REQUIRE((util::parse_uenv_string("prgenv-gnu") ==
           make_desc("prgenv-gnu", nullptr, nullptr, nullptr)));

  REQUIRE((util::parse_uenv_string("prgenv-gnu/23.2") ==
           make_desc("prgenv-gnu", "23.2", nullptr, nullptr)));

  REQUIRE((util::parse_uenv_string("prgenv-gnu/23.2:v2-rc1") ==
           make_desc("prgenv-gnu", "23.2", "v2-rc1", nullptr)));

  REQUIRE((util::parse_uenv_string("prgenv-gnu:default") ==
           make_desc("prgenv-gnu", nullptr, "default", nullptr)));

  REQUIRE((
      util::parse_uenv_string(
          "1736b4bb5ad9b3c5cae8878c71782a8bf2f2f739dbce8e039b629de418cb4dab") ==
      make_desc(
          nullptr, nullptr, nullptr,
          "1736b4bb5ad9b3c5cae8878c71782a8bf2f2f739dbce8e039b629de418cb4dab")));

  REQUIRE((util::parse_uenv_string("1736b4bb5ad9b3c5") ==
           make_desc(nullptr, nullptr, nullptr, "1736b4bb5ad9b3c5")));
}
