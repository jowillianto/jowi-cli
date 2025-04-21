module;
#include <format>
#include <string>
export module moderna.cli:app_version;

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