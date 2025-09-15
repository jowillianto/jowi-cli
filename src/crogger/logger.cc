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
  export struct logger {
  private:
    std::unique_ptr<context_filter<void>> __flt;
    std::unique_ptr<formatter<void>> __fmt;
    mutable std::unique_ptr<stream_emitter<void>> __emt;

    std::expected<void, log_error> __format(const context &ctx) const {
      auto it = std::back_inserter(*__emt);
      try {
        return __fmt->format(ctx, it);
      } catch (const log_error &e) {
        return std::unexpected{e};
      }
    }

  public:
    logger(size_t buf_size = 120) :
      __flt{std::make_unique<context_filter<no_filter>>()},
      __fmt{std::make_unique<formatter<colorful_formatter>>()},
      __emt{std::make_unique<stream_emitter<ss_emitter>>(buf_size, stdout_emitter{})} {}
    logger &set_filter(is_filter auto &&flt) {
      __flt = std::make_unique<context_filter<std::decay_t<decltype(flt)>>>(
        std::forward<decltype(flt)>(flt)
      );
      return *this;
    }

    logger &set_formatter(is_formatter auto &&fmt) {
      __fmt =
        std::make_unique<formatter<std::decay_t<decltype(fmt)>>>(std::forward<decltype(fmt)>(fmt));
      return *this;
    }

    logger &set_emitter(is_emitter auto &&emt, size_t buf_size = 120) {
      __emt =
        std::make_unique<stream_emitter<ss_emitter>>(buf_size, std::forward<decltype(emt)>(emt));
      return *this;
    }

    std::expected<void, log_error> log(const context &ctx) const {
      if (__flt->filter(ctx)) {
        return __format(ctx).and_then([&]() { return __emt->flush(); });
      }
      return {};
    }
  };
}