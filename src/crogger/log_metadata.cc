module;
#include <chrono>
#include <source_location>
#include <string>
export module jowi.crogger:log_metadata;
import :log_status;

namespace jowi::crogger {
  export struct log_metadata {
    log_status status;
    std::string message;
    std::source_location loc;
    std::chrono::system_clock::time_point time;

    static log_metadata test(std::source_location loc = std::source_location::current()) {
      return log_metadata{
        .status = log_status::critical(),
        .message = "Hello World",
        .loc = loc,
        .time = std::chrono::system_clock::now()
      };
    }
  };
}