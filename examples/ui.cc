import jowi.cli;
import jowi.cli.ui;
#include <print>

namespace cli = jowi::cli;
namespace ui = jowi::cli::ui;

ui::cli_nodes example_ui() {
  return ui::cli_nodes{
    /*
      Page Title
    */
    ui::cli_node::format_begin(
      ui::text_format{}.effect(ui::text_effect::bold).fg(ui::color::bright_blue())
    ),
    ui::cli_node::text("An Example User Interface"),
    ui::cli_node::format_end(),
    ui::cli_node::new_line(),
    /*
      Paragraph of Contents
    */
    ui::cli_node::text(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
    ),
  };
}

int main(int argc, const char **argv) {
  std::println("{}", example_ui());
}