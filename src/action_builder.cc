module;
#include <cstdlib>
#include <functional>
#include <string>
export module jowi.cli:action_builder;
import jowi.generic;
import :app;

namespace jowi::cli {
  struct AppAction {
    std::function<void(App &)> action;
    std::string help_text;

    void run(App &app_) const {
      action(app_);
    }
  };
  export struct ActionBuilder {
  private:
    generic::KeyVector<std::string, AppAction> __actions;
    std::reference_wrapper<App> __app;
    std::reference_wrapper<Arg> __arg;
    bool __arg_updated;
    uint64_t __id;

  public:
    ActionBuilder(App &app_, std::optional<std::string> help_text) :
      __app{std::ref(app_)}, __arg{std::ref(__app.get().add_argument())},
      __id{__app.get().parser().size() - 1} {
      if (help_text) {
        __arg.get().help(help_text.value());
      }
      __arg_updated = false;
    }

    template <class F>
      requires(std::is_invocable_r_v<void, F, App &>)
    ActionBuilder &add_action(std::string_view name, std::string help_text, F &&f) {
      __actions.emplace(name, AppAction{f, std::move(help_text)});
      __arg_updated = false;
      return *this;
    }

    ActionBuilder &update_args() {
      if (!__arg_updated) {
        auto options = ArgOptionsValidator{};
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
        app.error(1, "arg{}: {}", __id, ParseErrorType::NO_VALUE_GIVEN);
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
