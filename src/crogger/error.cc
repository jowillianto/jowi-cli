module;
#include <format>
export module jowi.crogger:error;
import jowi.generic;

namespace jowi::crogger {
  export enum struct LogErrorType { FORMAT_ERROR, IO_ERROR };
}

template <typename CharT> struct std::formatter<jowi::crogger::LogErrorType, CharT> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(jowi::crogger::LogErrorType type, auto &ctx) const {
    switch (type) {
      case jowi::crogger::LogErrorType::FORMAT_ERROR:
        return std::format_to(ctx.out(), "FORMAT_ERROR");
      case jowi::crogger::LogErrorType::IO_ERROR:
        return std::format_to(ctx.out(), "IO_ERROR");
    }
  }
};

namespace jowi::crogger {
  export struct LogError : public std::exception {
  private:
    LogErrorType __type;
    generic::FixedString<63> __msg;

  public:
    template <typename... Args>
      requires(std::formattable<Args, char> && ...)
    LogError(LogErrorType t, std::format_string<Args...> fmt, Args &&...args) noexcept : __type(t) {
      __msg.emplace_format("{}: ", t);
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
    }

    const char *what() const noexcept override {
      return __msg.c_str();
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static LogError format_error(std::format_string<Args...> fmt, Args &&...args) noexcept {
      return LogError{LogErrorType::FORMAT_ERROR, fmt, std::forward<Args>(args)...};
    }

    template <class... Args>
      requires(std::formattable<Args, char> && ...)
    static LogError io_error(std::format_string<Args...> fmt, Args &&...args) noexcept {
      return LogError{LogErrorType::FORMAT_ERROR, fmt, std::forward<Args>(args)...};
    }
  };
}