module;
#include <algorithm>
#include <expected>
#include <format>
#include <optional>
#include <string_view>
export module moderna.cli:app_version;
import :arg_shortcut;
import :parse_error;
namespace moderna::cli {
  export struct app_version {
    size_t major;
    size_t minor;
    size_t patch;

    constexpr friend bool operator==(const app_version &l, const app_version &r) {
      return l.major == r.major && l.minor == r.minor && l.patch == r.patch;
    }
    constexpr friend bool operator<(const app_version &l, const app_version &r) {
      if (l.major < r.major) return true;
      else if (l.major == r.major && l.minor < r.minor)
        return true;
      else if (l.major == r.major && l.minor == r.minor)
        return l.patch < r.patch;
      return false;
    }
    constexpr friend bool operator>(const app_version &l, const app_version &r) {
      if (l.major > r.major) return true;
      else if (l.major == r.major && l.minor > r.minor)
        return true;
      else if (l.major == r.major && l.minor == r.minor)
        return l.patch > r.patch;
      return false;
    }

    constexpr static std::expected<app_version, parse_error> from_string(std::string_view v) {
      if (v.length() == 0)
        return std::unexpected{parse_error::invalid_value("'{}' is not a valid version string", v)};
      auto major_minor_dot_pos = std::ranges::find(v, '.');
      if (major_minor_dot_pos == v.end()) {
        return std::unexpected{parse_error::invalid_value("'{}' is not a valid version string", v)};
      }
      auto minor_patch_dot_pos = std::ranges::find(major_minor_dot_pos + 1, v.end(), '.');
      if (minor_patch_dot_pos == v.end()) {
        return std::unexpected{parse_error::invalid_value("'{}' is not a valid version string", v)};
      }
      auto major_num = std::string_view{v.begin(), major_minor_dot_pos};
      auto minor_num = std::string_view{major_minor_dot_pos + 1, minor_patch_dot_pos};
      auto patch_num = std::string_view{minor_patch_dot_pos + 1, v.end()};
      return parse_arg<long long int>(major_num).and_then([&](auto major) {
        return parse_arg<long long int>(minor_num).and_then([&](auto minor) {
          return parse_arg<long long int>(patch_num).and_then([&](auto patch) {
            if (major < 0 || minor < 0 || patch < 0) {
              return std::expected<app_version, parse_error>{
                std::unexpected{parse_error::invalid_value("'{}' is not a valid version string", v)}
              };
            } else {
              return std::expected<app_version, parse_error>{app_version{
                static_cast<size_t>(major), static_cast<size_t>(minor), static_cast<size_t>(patch)
              }};
            }
          });
        });
      });
    }
  };

  template <> struct parse_shortcut<app_version> {
    std::expected<app_version, parse_error> from(std::string_view v) {
      return app_version::from_string(v);
    }
  };

  template auto parse_arg<app_version>(std::string_view);
}

namespace cli = moderna::cli;

template <class char_type> struct std::formatter<cli::app_version, char_type> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }
  constexpr auto format(const cli::app_version &v, auto &ctx) const {
    return std::format_to(ctx.out(), "{}.{}.{}", v.major, v.minor, v.patch);
  }
};