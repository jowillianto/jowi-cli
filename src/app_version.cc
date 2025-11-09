module;
#include <algorithm>
#include <expected>
#include <format>
#include <string_view>
export module jowi.cli:app_version;
import :arg_shortcut;
import :parse_error;
namespace jowi::cli {
  export struct AppVersion {
    uint64_t major;
    uint64_t minor;
    uint64_t patch;

    constexpr friend std::strong_ordering operator<=>(const AppVersion &l, const AppVersion &r) =
      default;
    constexpr friend bool operator==(const AppVersion &l, const AppVersion &r) {
      return l.major == r.major && l.minor == r.minor && l.patch == r.patch;
    }

    constexpr friend bool operator<(const AppVersion &l, const AppVersion &r) {
      if (l.major < r.major) return true;
      else if (l.major == r.major && l.minor < r.minor)
        return true;
      else if (l.major == r.major && l.minor == r.minor)
        return l.patch < r.patch;
      return false;
    }

    constexpr static std::expected<AppVersion, ParseError> from_string(std::string_view v) {
      if (v.length() == 0)
        return std::unexpected{ParseError::invalid_value("'{}' is not a valid version string", v)};
      auto major_minor_dot_pos = std::ranges::find(v, '.');
      if (major_minor_dot_pos == v.end()) {
        return std::unexpected{ParseError::invalid_value("'{}' is not a valid version string", v)};
      }
      auto minor_patch_dot_pos = std::ranges::find(major_minor_dot_pos + 1, v.end(), '.');
      if (minor_patch_dot_pos == v.end()) {
        return std::unexpected{ParseError::invalid_value("'{}' is not a valid version string", v)};
      }
      auto major_num = std::string_view{v.begin(), major_minor_dot_pos};
      auto minor_num = std::string_view{major_minor_dot_pos + 1, minor_patch_dot_pos};
      auto patch_num = std::string_view{minor_patch_dot_pos + 1, v.end()};
      return parse_arg<long long int>(major_num).and_then([&](auto major) {
        return parse_arg<long long int>(minor_num).and_then([&](auto minor) {
          return parse_arg<long long int>(patch_num).and_then([&](auto patch) {
            if (major < 0 || minor < 0 || patch < 0) {
              return std::expected<AppVersion, ParseError>{
                std::unexpected{ParseError::invalid_value("'{}' is not a valid version string", v)}
              };
            } else {
              return std::expected<AppVersion, ParseError>{AppVersion{
                static_cast<uint64_t>(major),
                static_cast<uint64_t>(minor),
                static_cast<uint64_t>(patch)
              }};
            }
          });
        });
      });
    }
  };

  template <> struct ParseShortcut<AppVersion> {
    std::expected<AppVersion, ParseError> from(std::string_view v) {
      return AppVersion::from_string(v);
    }
  };

  template auto parse_arg<AppVersion>(std::string_view);
}

namespace cli = jowi::cli;

template <class CharType> struct std::formatter<cli::AppVersion, CharType> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::AppVersion &v, auto &ctx) const {
    return std::format_to(ctx.out(), "{}.{}.{}", v.major, v.minor, v.patch);
  }
};