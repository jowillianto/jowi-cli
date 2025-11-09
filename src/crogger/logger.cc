module;
#include <concepts>
#include <expected>
#include <iterator>
#include <memory>
#include <string>
export module jowi.crogger:logger;
import :emitter;
import :filter;
import :formatter;
import :error;
import :context;

namespace jowi::crogger {
  export struct Logger {
  private:
    std::unique_ptr<ContextFilter<void>> __flt;
    std::unique_ptr<Formatter<void>> __fmt;
    mutable std::unique_ptr<StreamEmitter<void>> __emt;

    std::expected<void, LogError> __format(const Context &ctx) const {
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
    Logger &set_filter(is_filter auto &&flt) {
      __flt = std::make_unique<ContextFilter<std::decay_t<decltype(flt)>>>(
        std::forward<decltype(flt)>(flt)
      );
      return *this;
    }

    Logger &set_formatter(is_formatter auto &&fmt) {
      __fmt =
        std::make_unique<Formatter<std::decay_t<decltype(fmt)>>>(std::forward<decltype(fmt)>(fmt));
      return *this;
    }

    Logger &set_emitter(is_emitter auto &&emt, uint64_t buf_size = 120) {
      __emt =
        std::make_unique<StreamEmitter<SsEmitter>>(buf_size, std::forward<decltype(emt)>(emt));
      return *this;
    }

    std::expected<void, LogError> log(const Context &ctx) const {
      if (__flt->filter(ctx)) {
        return __format(ctx).and_then([&]() { return __emt->flush(); });
      }
      return {};
    }
  };
}