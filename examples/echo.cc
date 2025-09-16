#include <format>
import jowi.cli;

namespace cli = jowi::cli;

int main(int argc, const char **argv) {
  auto app = cli::app{
    cli::app_identity{
      .name = "Echo", .author = "Jonathan Willianto", .version = cli::app_version{0, 0, 0}
    },
    argc,
    argv
  };

  app.add_argument("--echo").required();
  app.parse_args();

  app.out("{}", app.args().first_of("--echo").value());
}