module;
#include <algorithm>
#include <charconv>
#include <format>
#include <optional>
#include <string_view>
export module moderna.cli:app_version;

namespace moderna::cli {
  constexpr std::optional<int> parse_number(std::string_view v) noexcept {
    int x = 0;
    auto res = std::from_chars(v.begin(), v.end(), x);
    if (res.ec != std::errc{}) {
      return std::nullopt;
    }
    return x;
  }
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

    constexpr static std::optional<app_version> from_string(std::string_view v) {
      if (v.length() == 0) return std::nullopt;
      auto major_minor_dot_pos = std::ranges::find(v, '.');
      if (major_minor_dot_pos == v.end()) {
        return std::nullopt;
      }
      auto minor_patch_dot_pos = std::ranges::find(major_minor_dot_pos + 1, v.end(), '.');
      if (minor_patch_dot_pos == v.end()) {
        return std::nullopt;
      }
      auto major_num = std::string_view{v.begin(), major_minor_dot_pos};
      auto minor_num = std::string_view{major_minor_dot_pos + 1, minor_patch_dot_pos};
      auto patch_num = std::string_view{minor_patch_dot_pos + 1, v.end()};
      return parse_number(major_num).and_then([&](int major) {
        return parse_number(minor_num).and_then([&](int minor) {
          return parse_number(patch_num).and_then([&](int patch) {
            if (major < 0 || minor < 0 || patch < 0) {
              return std::optional<app_version>{std::nullopt};
            } else {
              return std::optional{app_version{
                static_cast<size_t>(major), static_cast<size_t>(minor), static_cast<size_t>(patch)
              }};
            }
          });
        });
      });
    }
  };
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