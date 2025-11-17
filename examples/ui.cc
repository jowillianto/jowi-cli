import jowi.cli;
import jowi.tui;
#include <print>

namespace cli = jowi::cli;
namespace tui = jowi::tui;

int main() {
  static_cast<void>(tui::out_terminal.render(
    tui::DomNode::vstack(
      tui::Layout{}
        .append_child(
          tui::DomNode::vstack(
            tui::Layout{}
              .style(tui::DomStyle{}.effect(tui::TextEffect::BOLD).fg(tui::RgbColor::bright_blue()))
              .append_child(tui::DomNode{tui::Paragraph{"An Example User Interface"}})
          )
        )
        .append_child(
          tui::DomNode::paragraph(
            "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
            "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
            "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
            "dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
            "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
            "mollit anim id est laborum."
          )
        )
    )
  ));
}
