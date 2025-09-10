import moderna.cli;
#include <print>

namespace cli = moderna::cli;

cli::cli_nodes example_ui() {
  return cli::cli_nodes{
    /*
      Page Title
    */
    cli::cli_node::format_begin(
      cli::text_format{}.effect(cli::text_effect::bold).fg(cli::color::bright_blue())
    ),
    cli::cli_node::text("An Example User Interface"),
    cli::cli_node::format_end(),
    cli::cli_node::new_line(),
    /*
      Paragraph of Contents
    */
    cli::cli_node::text(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
    ),
  };
}

int main(int argc, const char **argv) {
  std::println("{}", example_ui());
}