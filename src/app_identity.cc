module;
#include <format>
#include <optional>
#include <string>
export module jowi.cli:app_identity;
import :app_version;

namespace jowi::cli {
  export struct app_identity {
    std::string name;
    std::string description;
    std::optional<std::string> author = std::nullopt;
    std::optional<std::string> license = std::nullopt;
    app_version version;
  };
}

namespace cli = jowi::cli;

template <class char_type> struct std::formatter<cli::app_identity, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  auto format(const cli::app_identity &v, auto &ctx) const {
    std::format_to(ctx.out(), "{} v{}\n", v.name, v.version);
    std::format_to(ctx.out(), "{}\n", v.description);
    if (v.author) std::format_to(ctx.out(), "Written by : {}\n", *v.author);
    if (v.license) std::format_to(ctx.out(), "{}\n", *v.license);
    return ctx.out();
  }
};