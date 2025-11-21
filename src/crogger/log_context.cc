module;
#include <chrono>
#include <format>
#include <source_location>
#include <string_view>
export module jowi.crogger:log_context;
import :log_level;
import :emitter;

namespace jowi::crogger {
  export struct RawMessage {
  public:
    virtual ~RawMessage() = default;
    virtual void format(std::back_insert_iterator<StreamEmitter<void>> &it) const = 0;
  };

  // Concrete implementation that holds format string and arguments
  export template <typename... Args> struct Message : public RawMessage {
  private:
    std::string_view __fmt;
    std::tuple<Args...> __args;

  public:
    Message(std::format_string<Args...> fmt, Args... arguments) :
      __fmt(fmt.get()), __args(std::forward<Args>(arguments)...) {}

    void format(std::back_insert_iterator<StreamEmitter<void>> &it) const override {
      std::apply(
        [&](const auto &...args) { std::vformat_to(it, __fmt, std::make_format_args(args...)); },
        __args
      );
    }
  };

  /*
    Context
    contains the logging Context, but this object should be consumed immediately after creation.
    This object should not be stored.
  */
  export struct LogContext {
    LogLevel status;
    std::source_location loc;
    std::chrono::system_clock::time_point time;
    const RawMessage &message;
  };
}
