module;
#include <chrono>
#include <expected>
#include <source_location>
export module jowi.crogger;
export import :context;
export import :emitter;
export import :error;
export import :filter;
export import :formatter;
export import :logger;
export import :severity;

/*
  Static Variables and usage
*/

namespace jowi::crogger {
  static logger root_logger{};

  export logger &root() {
    return root_logger;
  }

  export template <class... Args>
  std::expected<void, log_error> log(
    const logger &l,
    severity status,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    auto t = std::chrono::system_clock::now();
    return l.log(context{status, loc, t, fmt});
  }

  export template <class... Args>
  std::expected<void, log_error> log(
    severity status,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(root(), status, fmt, loc);
  }
  export template <class... Args>
  std::expected<void, log_error> trace(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::trace(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> debug(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::debug(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> info(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::info(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> warn(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::warn(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> error(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::error(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> critical(
    const logger &l,
    const message<Args...> &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, severity::critical(), fmt, loc);
  }

  // Global shortcuts (using root_logger)
  export template <class... Args>
  std::expected<void, log_error> trace(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::trace(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> debug(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::debug(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> info(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::info(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> warn(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::warn(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> error(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::error(), fmt, loc);
  }

  export template <class... Args>
  std::expected<void, log_error> critical(
    const message<Args...> &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(severity::critical(), fmt, loc);
  }
}