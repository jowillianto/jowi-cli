module;
#include <format>
export module jowi.crogger:error;
import jowi.generic;

namespace jowi::crogger {
  export enum struct LogErrorType {
    format_error,
    io_error,
  };
}

template <typename CharT> struct std::formatter<jowi::crogger::LogErrorType, CharT> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(jowi::crogger::LogErrorType type, auto &ctx) const {
    switch (type) {
      case jowi::crogger::LogErrorType::format_error:
        return std::format_to(ctx.out(), "format_error");
      case jowi::crogger::LogErrorType::io_error:
        return std::format_to(ctx.out(), "io_error");
    }
  }
};

namespace jowi::crogger {
  export struct LogError : public std::exception {
  private:
    LogErrorType __type;
    generic::fixed_string<60> __msg;

  public:
    template <typename... Args>
      requires(std::formattable<Args, char> && ...)
    LogError(LogErrorType t, std::format_string<Args...> fmt, Args &&...args) noexcept :
      __type(t) {
      __msg.emplace_format("{}: ", t);
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
    }

    const char *what() const noexcept override {
      return __msg.c_str();
    }
  };
}