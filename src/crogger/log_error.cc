module;
#include <format>
export module jowi.crogger:log_error;
import jowi.generic;

namespace jowi::crogger {
  export enum struct log_error_type {
    format_error,
    io_error,
  };
}

template <typename CharT> struct std::formatter<jowi::crogger::log_error_type, CharT> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(jowi::crogger::log_error_type type, auto &ctx) const {
    switch (type) {
      case jowi::crogger::log_error_type::format_error:
        return std::format_to(ctx.out(), "format_error");
      case jowi::crogger::log_error_type::io_error:
        return std::format_to(ctx.out(), "io_error");
    }
  }
};

namespace jowi::crogger {
  export struct log_error : public std::exception {
  private:
    log_error_type __type;
    generic::fixed_string<64> __msg;

  public:
    template <typename... Args>
      requires(std::formattable<Args, char> && ...)
    log_error(log_error_type t, std::format_string<Args...> fmt, Args &&...args) noexcept :
      __type(t) {
      __msg.emplace_format("{}: ", t);
      __msg.emplace_format(fmt, std::forward<Args>(args)...);
    }

    const char *what() const noexcept override {
      return __msg.c_str();
    }
  };
}