import jowi.test_lib;
import jowi.cli;

namespace cli = jowi::cli;
namespace test_lib = jowi::test_lib;

#include <jowi/test_lib.hpp>
#include <array>

static auto app_id = cli::app_identity{.name = "Random App", .version = cli::app_version{0, 0, 0}};

JOWI_ADD_TEST(test_read_positional_args) {
  std::array argv = {"some_exec", "--output", "10"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--output").require_value();
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("--output"));
  auto num_res = app.args().first_of("--output").transform(cli::parse_arg<int>).value();
  test_lib::assert_expected(num_res);
}

JOWI_ADD_TEST(test_read_flag) {
  std::array argv = {"some_exec", "--with_vibe"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--with_vibe");
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("--with_vibe"));
}

JOWI_ADD_TEST(test_double_convert) {
  std::array argv = {"some_exec", "--value", "10.2332323"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("--value").require_value();
  app.parse_args(false, false);
  auto num_res = app.args().first_of("--value").transform(cli::parse_arg<double>).value();
  test_lib::assert_expected(num_res);
  test_lib::assert_close(num_res.value(), 10.2332323);
}

JOWI_ADD_TEST(test_default_arg_is_flag) {
  std::array argv = {"some_exec", "-v"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v");
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("-v"));
}

JOWI_ADD_TEST(test_default_arg_required) {
  std::array argv = {"some_exec", "-v", "1"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v").required();
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("-v"));
  test_lib::assert_expected(app.args().first_of("-v").transform(cli::parse_arg<int>).value());
}

JOWI_ADD_TEST(test_multiple_arg_allowed) {
  std::array argv = {"some_exec", "-v", "1", "-v", "2"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v").required().n_at_least(1);
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("-v"));
  test_lib::assert_equal(app.args().count("-v"), 2);
  test_lib::assert_expected(app.args().first_of("-v").transform(cli::parse_arg<int>).value());
}

JOWI_ADD_TEST(test_constraint_three_arg) {
  std::array argv = {"some_exec", "-v", "1", "-v", "2", "-v", "3"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v").required().n_equal_to(3);
  app.parse_args(false, false);
  test_lib::assert_true(app.args().contains("-v"));
  test_lib::assert_equal(app.args().count("-v"), 3);
  test_lib::assert_expected(app.args().first_of("-v").transform(cli::parse_arg<int>).value());
}

JOWI_ADD_TEST(test_required_imply_not_flag) {
  std::array argv = {"some_exec", "-v"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v").required();
  auto res = app.parse_args(false, false);
  test_lib::assert_false(res.has_value());
}

JOWI_ADD_TEST(test_required_imply_single_arg) {
  std::array argv = {"some_exec", "-v", "1", "-v", "2"};
  auto app = cli::app{app_id, argv.size(), argv.data()};
  app.add_argument("-v").required();
  auto res = app.parse_args(false, false);
  test_lib::assert_false(res.has_value());
}