module;
#include <cstdlib>
#include <functional>
#include <string>
export module jowi.cli:action_builder;
import jowi.generic;
import :app;

namespace jowi::cli {
  struct app_action {
    std::function<void(app &)> action;
    std::string help_text;

    void run(app &app_) const {
      action(app_);
    }
  };
  export struct action_builder {
  private:
    generic::key_vector<std::string, app_action> __actions;
    std::reference_wrapper<app> __app;
    std::reference_wrapper<arg> __arg;
    bool __arg_updated;
    uint64_t __id;

  public:
    action_builder(app &app_, std::optional<std::string> help_text) :
      __app{std::ref(app_)}, __arg{std::ref(__app.get().add_argument())},
      __id{__app.get().parser().size() - 1} {
      if (help_text) {
        __arg.get().help(help_text.value());
      }
      __arg_updated = false;
    }

    template <class F>
      requires(std::is_invocable_r_v<void, F, app &>)
    action_builder &add_action(std::string_view name, std::string help_text, F &&f) {
      __actions.emplace(name, app_action{f, std::move(help_text)});
      __arg_updated = false;
      return *this;
    }

    action_builder &update_args() {
      if (!__arg_updated) {
        auto options = arg_options_validator{};
        for (const auto &[n, a] : __actions) {
          options.add_option(n, a.help_text);
        }
        __arg.get().add_validator(std::move(options));
        __arg_updated = true;
      }
      return *this;
    }

    void run() {
      auto &app = __app.get();
      update_args();
      app.parse_args(false);
      if (app.args().size() - 1 != __id) {
        if (app.args().contains("-h") || app.args().contains("--help")) {
          app.out(0, "{}", app.help());
        }
        app.error(1, "arg{}: {}", __id, parse_error_type::NO_VALUE_GIVEN);
      }
      auto arg_value = app.args().arg();
      auto action = __actions.get(arg_value);
      if (!action) {
        std::exit(1);
      } else {
        action->get().run(app);
      }
    }
  };
}