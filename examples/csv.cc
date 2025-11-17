import jowi.cli;
import jowi.tui;
#include <filesystem>
#include <fstream>
#include <print>

namespace cli = jowi::cli;
namespace tui = jowi::tui;
namespace fs = std::filesystem;

int main(int argc, const char **argv) {
  cli::App app{
    cli::AppIdentity{
      .name = "CSV Reader",
      .description = "A Simple to use csv reader",
      .version = cli::AppVersion{0, 0, 0}
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
    auto render_line = [&](tui::DomStyle style) {
      auto layout = tui::Layout{};
      layout.style(std::move(style));
      layout.append_child(tui::DomNode::paragraph(line));
      static_cast<void>(tui::out_terminal.render(tui::DomNode::vstack(std::move(layout))));
    };

    if (is_first_line && headers) {
      render_line(
        tui::DomStyle{}.effect(tui::TextEffect::BOLD)
          .effect(tui::TextEffect::UNDERLINE)
          .fg(tui::RgbColor::bright_blue())
      );
      is_first_line = false;
    } else {
      render_line(tui::DomStyle{});
    }
  }
}
