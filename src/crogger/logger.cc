module;
#include <chrono>
#include <concepts>
#include <expected>
#include <iterator>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
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
    mutable std::unique_ptr<StreamEmitter<void>> __emt;

    std::expected<void, LogError> __format(const LogContext &ctx) const {
      auto it = std::back_inserter(*__emt);
      try {
        return __fmt->format(ctx, it);
      } catch (const LogError &e) {
        return std::unexpected{e};
      }
    }

  public:
    Logger(uint64_t buf_size = 120) :
      __flt{std::make_unique<ContextFilter<NoFilter>>()},
      __fmt{std::make_unique<Formatter<ColorfulFormatter>>()},
      __emt{std::make_unique<StreamEmitter<SsEmitter>>(buf_size, StdoutEmitter{})} {}
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

    Logger &set_emitter(IsEmitter auto &&emt, uint64_t buf_size = 256) {
      __emt =
        std::make_unique<StreamEmitter<SsEmitter>>(buf_size, std::forward<decltype(emt)>(emt));
      return *this;
    }

    void log(const LogContext &ctx) const {
      if (__flt->filter(ctx)) {
        auto it = std::back_inserter(*__emt);
        __fmt->format(ctx, it)
          .and_then([&]() { return __emt->flush(); })
          .or_else([&](auto &&e) {
            auto formatter = ColorfulFormatter{};
            auto emitter = StreamEmitter<SsEmitter>{256, StderrEmitter{}};
            auto it = std::back_inserter<StreamEmitter<void>>(emitter);
            static_cast<void>(formatter.format(ctx, it));
            LogContext err_ctx{
              LogLevel::error(),
              std::source_location::current(),
              std::chrono::system_clock::now(),
              Message{"{}", e.what()}
            };
            static_cast<void>(formatter.format(err_ctx, it));
            return std::expected<void, LogError>{};
          })
          .value();
      }
    }
  };
}