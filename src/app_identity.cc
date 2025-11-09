module;
#include <format>
#include <optional>
#include <string>
export module jowi.cli:app_identity;
import :app_version;

namespace jowi::cli {
  export struct AppIdentity {
    std::string name;
    std::string description;
    std::optional<std::string> author = std::nullopt;
    std::optional<std::string> license = std::nullopt;
    AppVersion version;
  };
}

namespace cli = jowi::cli;

template <class CharType> struct std::formatter<cli::AppIdentity, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  auto format(const cli::AppIdentity &v, auto &ctx) const {
    std::format_to(ctx.out(), "{} v{}\n", v.name, v.version);
    std::format_to(ctx.out(), "{}\n", v.description);
    if (v.author) std::format_to(ctx.out(), "Written by : {}\n", *v.author);
    if (v.license) std::format_to(ctx.out(), "{}\n", *v.license);
    return ctx.out();
  }
};