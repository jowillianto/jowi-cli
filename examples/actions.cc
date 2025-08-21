#include <format>
import moderna.cli;

namespace cli = moderna::cli;

int main(int argc, const char **argv) {
  auto app = cli::app{
    cli::app_identity{
      .name = "Actions", .author = "Jonathan Willianto", .version = cli::app_version{0, 0, 0}
    },
    argc,
    argv
  };

  cli::action_builder{app, "The main action to perform"}
    .add_action(
      "echo",
      "echo app from the previous one",
      [](cli::app &app) {
        app.add_argument("--echo").help("The argument to echo").required();
        app.parse_args();
        app.out("{}", app.args().first_of("--echo").value());
      }
    )
    .add_action(
      "port",
      "Checks a port number",
      [](cli::app &app) {
        app.add_argument("--port").help("The port number to use").required();
        app.parse_args();
        int port = app.expect(app.args().first_of("--port").transform(cli::parse_arg<int>).value());
        app.out("Port: {}", port);
      }
    )
    .run();
}