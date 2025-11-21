#include <format>
#include <print>
import jowi.cli;

namespace cli = jowi::cli;

int main(int argc, const char **argv) {
  auto app = cli::App{
    cli::AppIdentity{
      .name = "Echo", .author = "Jonathan Willianto", .version = cli::AppVersion{0, 0, 0}
    },
    argc,
    argv
  };

  app.add_argument("--echo").help("The string to echo out").with_default("Hello World").required();
  app.parse_args();

  std::println("{}", app.args().first_of("--echo").value());
}