#include <vector>

#include <catch_amalgamated.hpp>

#include <lib/strings.hpp>

TEST_CASE("split strings", "[strings]") {
  using v = std::vector<std::string>;

  REQUIRE(util::split("", ',') == v({""}));
  REQUIRE(util::split(",", ',') == v({"", ""}));
  REQUIRE(util::split("a,", ',') == v({"a", ""}));
  REQUIRE(util::split("", ',') == v({""}));
  REQUIRE(util::split(",", ',') == v({"", ""}));
  REQUIRE(util::split(",,", ',') == v({"", "", ""}));
  REQUIRE(util::split(",a", ',') == v({"", "a"}));
  REQUIRE(util::split("a,", ',') == v({"a", ""}));
  REQUIRE(util::split("a", ',') == v({"a"}));
  REQUIRE(util::split("a,b", ',') == v({"a", "b"}));
  REQUIRE(util::split("a,b,c", ',') == v({"a", "b", "c"}));
  REQUIRE(util::split("a,b,,c", ',') == v({"a", "b", "", "c"}));
}

TEST_CASE("split strings dropping empty", "[strings]") {
  using v = std::vector<std::string>;

  REQUIRE(util::split("", ',', true) == v({}));
  REQUIRE(util::split(",", ',', true) == v({}));
  REQUIRE(util::split("a,", ',', true) == v({"a"}));
  REQUIRE(util::split("", ',', true) == v({}));
  REQUIRE(util::split(",", ',', true) == v({}));
  REQUIRE(util::split(",,", ',', true) == v({}));
  REQUIRE(util::split(",a", ',', true) == v({"a"}));
  REQUIRE(util::split("a,", ',', true) == v({"a"}));
  REQUIRE(util::split("a", ',', true) == v({"a"}));
  REQUIRE(util::split("a,b", ',', true) == v({"a", "b"}));
  REQUIRE(util::split("a,b,c", ',', true) == v({"a", "b", "c"}));
  REQUIRE(util::split("a,b,,c", ',', true) == v({"a", "b", "c"}));
}

TEST_CASE("valid sha length 64", "[strings]") {
  REQUIRE(util::is_sha(
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
  REQUIRE(util::is_sha(
      "1234567890abcdefABCDEFaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
}

TEST_CASE("invalid sha length 64", "[strings]") {
  REQUIRE(!util::is_sha(
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  REQUIRE(!util::is_sha(
      "----------------------------------------------------------------"));
  REQUIRE(!util::is_sha(
      "gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg"));
  REQUIRE(!util::is_sha(
      "1234567890abcdefABCDEFaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax"));
}

TEST_CASE("valid sha length 16 (id)", "[strings]") {
  REQUIRE(util::is_sha("0123456789abcdef"));
  REQUIRE(util::is_sha("0123456789ABCDEF"));
}

TEST_CASE("invalid sha length 16 (id)", "[strings]") {
  REQUIRE(!util::is_sha("0123456789abcdeg"));
  REQUIRE(!util::is_sha("0123456789A-CDEF"));
}
