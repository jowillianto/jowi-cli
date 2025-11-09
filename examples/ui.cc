import jowi.cli;
import jowi.tui;
#include <print>

namespace cli = jowi::cli;
namespace tui = jowi::tui;

tui::CliNodes example_ui() {
  return tui::CliNodes{
    /*
      Page Title
    */
    tui::CliNode::format_begin(
      tui::TextFormat{}.effect(tui::TextEffect::bold).fg(tui::Color::bright_blue())
    ),
    tui::CliNode::text("An Example User Interface"),
    tui::CliNode::format_end(),
    tui::CliNode::new_line(),
    /*
      Paragraph of Contents
    */
    tui::CliNode::text(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
    ),
  };
}

int main(int argc, const char **argv) {
  std::println("{}", example_ui());
}
