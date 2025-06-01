import moderna.test_lib;
import moderna.cli;
#include <moderna/test_lib.hpp>
#include <cstddef>
#include <limits>
#include <format>
#include <string>

namespace cli = moderna::cli;
namespace test_lib = moderna::test_lib;

MODERNA_ADD_TEST(cli_version_init_test) {
  auto major = static_cast<size_t>(test_lib::random_integer(0, 100));
  auto minor = static_cast<size_t>(test_lib::random_integer(0, 100));
  auto patch = static_cast<size_t>(test_lib::random_integer(0, 100));
  cli::app_version version{major, minor, patch};
  test_lib::assert_equal(major, version.major);
  test_lib::assert_equal(minor, version.minor);
  test_lib::assert_equal(patch, version.patch);
}
MODERNA_ADD_TEST(cli_version_eq_test) {
  cli::app_version v1{1, 2, 3};
  cli::app_version v2{1, 2, 3};
  cli::app_version v3{1, 2, 4};
  test_lib::assert_true(v1 == v2);
  test_lib::assert_false(v1 == v3);
}

MODERNA_ADD_TEST(cli_version_less_than_test) {
  test_lib::assert_true(cli::app_version{1, 0, 0} < cli::app_version{2, 0, 0});
  test_lib::assert_true(cli::app_version{1, 2, 0} < cli::app_version{1, 3, 0});
  test_lib::assert_true(cli::app_version{1, 2, 3} < cli::app_version{1, 2, 4});
  test_lib::assert_false(cli::app_version{2, 0, 0} < cli::app_version{1, 0, 0});
}

MODERNA_ADD_TEST(cli_version_greater_than_test) {
  test_lib::assert_true(cli::app_version{2, 0, 0} > cli::app_version{1, 0, 0});
  test_lib::assert_true(cli::app_version{1, 3, 0} > cli::app_version{1, 2, 0});
  test_lib::assert_true(cli::app_version{1, 2, 4} > cli::app_version{1, 2, 3});
  test_lib::assert_false(cli::app_version{1, 0, 0} > cli::app_version{2, 0, 0});
}

MODERNA_ADD_TEST(cli_version_parse_valid_test) {
  auto maybe_version = cli::app_version::from_string("1.2.3");
  test_lib::assert_true(maybe_version.has_value());
  auto version = *maybe_version;
  test_lib::assert_equal(version.major, 1);
  test_lib::assert_equal(version.minor, 2);
  test_lib::assert_equal(version.patch, 3);
}

MODERNA_ADD_TEST(cli_version_parse_invalid_test) {
  test_lib::assert_false(cli::app_version::from_string("").has_value());
  test_lib::assert_false(cli::app_version::from_string("1.2").has_value());
  test_lib::assert_false(cli::app_version::from_string("1..3").has_value());
  test_lib::assert_false(cli::app_version::from_string("a.b.c").has_value());
  test_lib::assert_false(cli::app_version::from_string("1.2.-3").has_value());
}

MODERNA_ADD_TEST(cli_version_parse_leading_zero_test) {
  auto maybe_version = cli::app_version::from_string("01.02.03");
  test_lib::assert_true(maybe_version.has_value());
  auto version = *maybe_version;
  test_lib::assert_equal(version.major, 1);
  test_lib::assert_equal(version.minor, 2);
  test_lib::assert_equal(version.patch, 3);
}

MODERNA_ADD_TEST(cli_version_format_test) {
  cli::app_version version{4, 5, 6};
  auto formatted = std::format("{}", version);
  test_lib::assert_equal(formatted, "4.5.6");
}

MODERNA_ADD_TEST(cli_version_parse_and_format_roundtrip) {
  std::string_view version_str = "10.20.30";
  auto maybe_version = cli::app_version::from_string(version_str);
  test_lib::assert_true(maybe_version.has_value());
  auto formatted = std::format("{}", *maybe_version);
  test_lib::assert_equal(formatted, version_str);
}

MODERNA_ADD_TEST(cli_version_parse_max_int_test) {
  int max_int = std::numeric_limits<int>::max();
  std::string version_str = std::format("{}.{}.{}", max_int, max_int, max_int);
  auto maybe_version = cli::app_version::from_string(version_str);
  test_lib::assert_true(maybe_version.has_value());
  auto version = *maybe_version;
  test_lib::assert_equal(version.major, static_cast<size_t>(max_int));
  test_lib::assert_equal(version.minor, static_cast<size_t>(max_int));
  test_lib::assert_equal(version.patch, static_cast<size_t>(max_int));
}

MODERNA_ADD_TEST(cli_version_parse_overflow_test) {
  std::string version_str = "999999999999.1.1"; // too large for int
  auto maybe_version = cli::app_version::from_string(version_str);
  test_lib::assert_false(maybe_version.has_value());
}

MODERNA_ADD_TEST(cli_version_parse_negative_test) {
  auto maybe_version = cli::app_version::from_string("-1.0.0");
  test_lib::assert_false(maybe_version.has_value());

  maybe_version = cli::app_version::from_string("1.-1.0");
  test_lib::assert_false(maybe_version.has_value());

  maybe_version = cli::app_version::from_string("1.0.-1");
  test_lib::assert_false(maybe_version.has_value());
}

MODERNA_ADD_TEST(cli_version_parse_random_strings_test) {
  constexpr int iterations = 100;
  for (int i = 0; i < iterations; ++i) {
    auto random_str = test_lib::random_string(test_lib::random_integer(0, 20));
    auto result = cli::app_version::from_string(random_str);
    // It’s enough that this doesn't crash or throw; no assertion required
    // Optionally, print something for debugging
    // std::println("Input: '{}', Parsed: {}", random_str, result.has_value());
  }
  test_lib::assert_true(true);  // Always pass, this is a stability test
}

MODERNA_ADD_TEST(cli_version_parse_valid_random_numbers_test) {
  for (int i = 0; i < 50; ++i) {
    int major = test_lib::random_integer(0, 10000);
    int minor = test_lib::random_integer(0, 10000);
    int patch = test_lib::random_integer(0, 10000);
    std::string version_str = std::format("{}.{}.{}", major, minor, patch);
    auto maybe_version = cli::app_version::from_string(version_str);
    test_lib::assert_true(maybe_version.has_value());
    auto version = *maybe_version;
    test_lib::assert_equal(version.major, static_cast<size_t>(major));
    test_lib::assert_equal(version.minor, static_cast<size_t>(minor));
    test_lib::assert_equal(version.patch, static_cast<size_t>(patch));
  }
}