module;
#include <chrono>
#include <expected>
#include <source_location>
export module jowi.crogger;
export import :log_context;
export import :emitter;
export import :error;
export import :filter;
export import :formatter;
export import :logger;
export import :log_level;

/*
  Static Variables and usage
*/

namespace jowi::crogger {
  static Logger root_logger{};

  export Logger &root() {
    return root_logger;
  }

  export void log(
    const Logger &l,
    LogLevel status,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    auto t = std::chrono::system_clock::now();
    return static_cast<void>(l.log(LogContext{status, loc, t, fmt}));
  }

  export void log(
    LogLevel status,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(root(), status, fmt, loc);
  }
  export void trace(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::trace(), fmt, loc);
  }

  export void debug(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::debug(), fmt, loc);
  }

  export void info(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::info(), fmt, loc);
  }

  export void warn(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::warn(), fmt, loc);
  }

  export void error(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::error(), fmt, loc);
  }

  export void critical(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, LogLevel::critical(), fmt, loc);
  }

  // Global shortcuts (using root_logger)
  export void trace(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::trace(), fmt, loc);
  }

  export void debug(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::debug(), fmt, loc);
  }

  export void info(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::info(), fmt, loc);
  }

  export void warn(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::warn(), fmt, loc);
  }

  export void error(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::error(), fmt, loc);
  }

  export void critical(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(LogLevel::critical(), fmt, loc);
  }
}