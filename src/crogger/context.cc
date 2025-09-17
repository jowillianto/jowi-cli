module;
#include <chrono>
#include <format>
#include <source_location>
#include <string_view>
export module jowi.crogger:context;
import :severity;
import :emitter;

namespace jowi::crogger {
  export struct raw_message {
  public:
    virtual ~raw_message() = default;
    virtual void format(std::back_insert_iterator<stream_emitter<void>> &it) const = 0;
  };

  // Concrete implementation that holds format string and arguments
  export template <typename... Args> struct message : public raw_message {
  private:
    std::string_view __fmt;
    std::tuple<Args...> __args;

  public:
    message(std::format_string<Args...> fmt, Args... arguments) :
      __fmt(fmt.get()), __args(std::forward<Args>(arguments)...) {}

    void format(std::back_insert_iterator<stream_emitter<void>> &it) const override {
      std::apply(
        [&](const auto &...args) { std::vformat_to(it, __fmt, std::make_format_args(args...)); },
        __args
      );
    }
  };

  /*
    this should mirror message.
  */
  export template <typename... Args> struct msg : public raw_message {
  private:
    std::string_view __fmt;
    std::tuple<Args...> __args;

  public:
    msg(std::format_string<Args...> fmt, Args... arguments) :
      __fmt(fmt.get()), __args(std::forward<Args>(arguments)...) {}

    void format(std::back_insert_iterator<stream_emitter<void>> &it) const override {
      std::apply(
        [&](const auto &...args) { std::vformat_to(it, __fmt, std::make_format_args(args...)); },
        __args
      );
    }
  };

  /*
    context
    contains the logging context, but this object should be consumed immediately after creation.
    This object should not be stored.
  */
  export struct context {
    severity status;
    std::source_location loc;
    std::chrono::system_clock::time_point time;
    const raw_message &message;
  };
}