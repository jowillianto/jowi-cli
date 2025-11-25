module;
#include <chrono>
#include <expected>
#include <memory>
#include <source_location>
export module jowi.crogger:logger;
import :emitter;
import :filter;
import :formatter;
import :error;
import :log_context;

namespace jowi::crogger {
  export struct Logger {
  private:
    std::unique_ptr<ContextFilter<void>> __flt;
    std::unique_ptr<Formatter<void>> __fmt;
    mutable std::unique_ptr<Emitter<void>> __emt;

  public:
    Logger(uint64_t buf_size = 120) :
      __flt{std::make_unique<ContextFilter<NoFilter>>()},
      __fmt{std::make_unique<Formatter<ColorfulFormatter>>()},
      __emt{std::make_unique<Emitter<StdoutEmitter>>()} {}
    Logger &set_filter(IsFilter auto &&flt) {
      __flt = std::make_unique<ContextFilter<std::decay_t<decltype(flt)>>>(
        std::forward<decltype(flt)>(flt)
      );
      return *this;
    }

    Logger &set_formatter(IsFormatter auto &&fmt) {
      __fmt =
        std::make_unique<Formatter<std::decay_t<decltype(fmt)>>>(std::forward<decltype(fmt)>(fmt));
      return *this;
    }

    Logger &set_emitter(IsEmitter auto &&emt) {
      __emt =
        std::make_unique<Emitter<std::decay_t<decltype(emt)>>>(std::forward<decltype(emt)>(emt));
      return *this;
    }

    void log(const LogContext &ctx) const {
      if (__flt->filter(ctx)) {
        __fmt->format(ctx)
          .and_then([&](auto msg) { return __emt->emit(std::move(msg)); })
          .or_else([&](auto &&e) {
            return ColorfulFormatter{}
              .format(
                {LogLevel::error(),
                 std::source_location::current(),
                 std::chrono::system_clock::now(),
                 Message{"{}", e.what()}}
              )
              .and_then([&](auto msg) { return StdoutEmitter{}.emit(std::move(msg)); });
          })
          .value();
      }
    }
  };
}