import moderna.cli;
#include <filesystem>
#include <fstream>
#include <print>
#include <ranges>

namespace cli = moderna::cli;
namespace fs = std::filesystem;

int main(int argc, const char **argv) {
  cli::app app{
    cli::app_identity{
      .name = "CSV Reader",
      .description = "A Simple to use csv reader",
      .version = cli::app_version{0, 0, 0}
    },
    argc,
    argv
  };

  app.add_argument("--csv").help("The path to the csv file").required();
  app.add_argument("--headers").optional().as_flag().help("If the CSV file have headers");
  app.parse_args();

  auto csv_path = app.args().first_of("--csv").transform(cli::parse_arg<fs::path>).value();
  auto headers = app.args().contains("--headers");

  if (!fs::is_regular_file(csv_path)) {
    app.error(1, "{} is not a file", csv_path.c_str());
  }

  std::fstream csv_file{csv_path, std::ios_base::in};
  std::string line;
  bool is_first_line = true;
  while (std::getline(csv_file, line, '\n')) {
    auto comma_sep = std::ranges::transform_view(std::ranges::split_view(line, ','), [](auto &&s) {
      return cli::cli_node::text("{}", std::string_view{s.begin(), s.end()});
    });
    if (is_first_line && headers) {
      app.out(
        "{}",
        cli::cli_nodes{
          cli::cli_node::format_begin(
            cli::text_format{}
              .effect(cli::text_effect::bold)
              .effect(cli::text_effect::underline)
              .fg(cli::color::bright_blue())
          ),
          comma_sep,
          cli::cli_node::format_end()
        }
      );
      is_first_line = false;
    } else {
      app.out("{}", cli::cli_nodes{comma_sep});
    }
  }
}