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
  static Logger root_logger{};

  export Logger &root() {
    return root_logger;
  }

  export std::expected<void, LogError> log(
    const Logger &l,
    Severity status,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    auto t = std::chrono::system_clock::now();
    return l.log(Context{status, loc, t, fmt});
  }

  export std::expected<void, LogError> log(
    Severity status,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(root(), status, fmt, loc);
  }
  export std::expected<void, LogError> trace(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::trace(), fmt, loc);
  }

  export std::expected<void, LogError> debug(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::debug(), fmt, loc);
  }

  export std::expected<void, LogError> info(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::info(), fmt, loc);
  }

  export std::expected<void, LogError> warn(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::warn(), fmt, loc);
  }

  export std::expected<void, LogError> error(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::error(), fmt, loc);
  }

  export std::expected<void, LogError> critical(
    const Logger &l,
    const RawMessage &fmt,
    std::source_location loc = std::source_location::current()
  ) {
    return log(l, Severity::critical(), fmt, loc);
  }

  // Global shortcuts (using root_logger)
  export std::expected<void, LogError> trace(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::trace(), fmt, loc);
  }

  export std::expected<void, LogError> debug(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::debug(), fmt, loc);
  }

  export std::expected<void, LogError> info(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::info(), fmt, loc);
  }

  export std::expected<void, LogError> warn(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::warn(), fmt, loc);
  }

  export std::expected<void, LogError> error(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::error(), fmt, loc);
  }

  export std::expected<void, LogError> critical(
    const RawMessage &fmt, std::source_location loc = std::source_location::current()
  ) {
    return log(Severity::critical(), fmt, loc);
  }
}