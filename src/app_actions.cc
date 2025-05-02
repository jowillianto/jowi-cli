module;
#include <functional>
#include <string>
#include <ranges>
#include <algorithm>
export module moderna.cli:app_actions;
import :app;
import :argument;

namespace moderna::cli {
  export struct app_action {
    std::string option;
    std::string description;
    std::function<void(app &)> action;

    argument_option to_option() const {
      return argument_option{option, description};
    }
  };
  export class app_actions {
    std::vector<app_action> __actions;

    constexpr app_actions() {}

  public:
    /*
      adds an action into the app_actions list, if the actions already exist, this will OVERWRITE
      the previous action.
    */
    template <std::invocable<app &> F>
    constexpr app_actions &add_action(
      std::string_view option, std::string_view description, F &&action
    ) {
      auto action_it = std::ranges::find(__actions, option, &app_action::option);
      if (action_it == __actions.end()) {
        __actions.emplace_back(
          std::string{option}, std::string{description}, std::forward<F>(action)
        );
      } else {
        action_it->description = description;
        action_it->action = action;
      }
      return *this;
    }
    /*
      Gets an action from the app actions list.
    */
    std::optional<std::reference_wrapper<const app_action>> get_action(std::string_view option) {
      auto action = std::ranges::find(__actions, option, &app_action::option);
      if (action == __actions.end()) return std::nullopt;
      return std::cref(*action);
    }

    void apply_and_run(app &cli_app) {
      auto &arg = cli_app.add_argument(false).help("Action to perform");
      arg.options = std::vector<argument_option>{};
      arg.options->reserve(__actions.size());
      auto option_inserter = std::back_inserter(arg.options.value());
      std::ranges::transform(__actions, option_inserter, &app_action::to_option);
      cli_app.parse_argument({.is_final = false});
      auto arg_option = cli_app.arguments().current_positional();
      cli_app.add_help_argument();
      auto action = get_action(arg_option);
      if (!action) {
        cli_app.error(1, "Error: No such action \"{}\"", arg_option);
      } else {
        cli_app.help(action->get().description);
        action->get().action(cli_app);
      }
    }

    constexpr static app_actions empty() {
      return app_actions{};
    }
  };
}