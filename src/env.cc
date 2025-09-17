module;
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
export module jowi.cli:env;
import jowi.generic;
import :parse_error;

namespace jowi::cli {
  struct app_env {
  private:
    generic::key_vector<std::string, std::string> __env;

  public:
    app_env() {}

    /*
      get_env.
      Gets a key from the current environment.
    */
    template <generic::is_comparable<std::string> Key>
    std::optional<std::string_view> get_env(Key &&key) const noexcept {
      return __env.get(std::forward<Key>(key));
    }

    /*
      set_env
      This will only return parse_error if value or key contains =. If the input is sanitized
      such that it does not contain '=', then this error will not happen.
    */
    std::expected<std::string_view, parse_error> set_env(
      std::string_view key, std::string_view value
    ) {
      if (key.contains('=')) {
        return std::unexpected{parse_error::invalid_value("key {} contains '='", key)};
      }
      if (value.contains('=')) {
        return std::unexpected{parse_error::invalid_value("value {} contains '='", value)};
      }
      return __env.emplace(key, value);
    }

    void sync_to_sys() const {
      for (const auto &[k, v] : __env) {
        int res = setenv(k.c_str(), v.c_str(), 1);
        if (res == ENOMEM) {
          throw std::bad_alloc{};
        }
      }
    }

    constexpr auto begin() const {
      return __env.begin();
    }
    constexpr auto end() const {
      return __env.end();
    }

    uint64_t size() const {
      return __env.size();
    }

    static std::expected<app_env, parse_error> from_ptr(const char **envp) {
      const char *envp_entry = envp[0];
      app_env env;
      while (envp_entry != nullptr) {
        std::string_view entry{envp_entry};
        auto eq_pos = std::ranges::find(entry, '=');
        if (eq_pos == entry.end()) {
          return std::unexpected{parse_error::invalid_value("no '=' found for {}", entry)};
        }
        auto key = std::string_view{entry.begin(), eq_pos};
        auto value = std::string_view{eq_pos + 1, entry.end()};
        auto res = env.set_env(key, value);
        if (!res) {
          return std::unexpected{std::move(res.error())};
        }
      }
      return env;
    }
  };
}