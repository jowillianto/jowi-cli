#include <format>
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

  app.add_argument("--echo").required();
  app.parse_args();

  app.out("{}", app.args().first_of("--echo").value());
}