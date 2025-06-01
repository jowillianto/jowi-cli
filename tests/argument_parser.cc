import moderna.test_lib;
import moderna.cli;

namespace cli = moderna::cli;
namespace test_lib = moderna::test_lib;

#include <moderna/test_lib.hpp>
#include <array>
#include <print>

static auto app_id = cli::app_identity{.name{"Random App"}};

MODERNA_ADD_TEST(test_read_positional_args) {
  std::array argv = {"some_exec", "--output", "10"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--output");
  app.parse_argument();
  test_lib::assert_true(app.arguments().contains("--output"));
  auto num_res = app.arguments()
                   .first_of("--output")
                   .transform([](cli::argument_value &&v) { return v.cast<int>(); })
                   .value();
  test_lib::assert_expected(num_res);
}

MODERNA_ADD_TEST(test_read_flag) {
  std::array argv = {"some_exec", "--with_vibe"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--with_vibe");
  app.parse_argument();
  test_lib::assert_true(app.arguments().contains("--with_vibe"));
}

MODERNA_ADD_TEST(test_double_convert) {
  std::array argv = {"some_exec", "--value", "10.2332323"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--value");
  app.parse_argument();
  auto num_res = app.arguments()
                   .first_of("--value")
                   .transform([](cli::argument_value &&v) { return v.cast<double>(); })
                   .value();
  test_lib::assert_expected(num_res);
  test_lib::assert_close(num_res.value(), 10.2332323);
}