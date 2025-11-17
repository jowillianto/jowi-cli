#include <format>
#include <print>
import jowi.cli;

namespace cli = jowi::cli;

int main(int argc, const char **argv) {
  auto app = cli::App{
    cli::AppIdentity{
      .name = "Actions", .author = "Jonathan Willianto", .version = cli::AppVersion{0, 0, 0}
    },
    argc,
    argv
  };

  cli::ActionBuilder{app, "The main action to perform"}
    .add_action(
      "echo",
      "echo app from the previous one",
      [](cli::App &app) {
        app.add_argument("--echo").help("The argument to echo").required();
        app.parse_args();
        std::println("{}", app.args().first_of("--echo").value());
      }
    )
    .add_action(
      "port",
      "Checks a port number",
      [](cli::App &app) {
        app.add_argument("--port").help("The port number to use").required();
        app.parse_args();
        int port = app.expect(app.args().first_of("--port").transform(cli::parse_arg<int>).value());
        std::println("Port: {}", port);
      }
    )
    .run();
}