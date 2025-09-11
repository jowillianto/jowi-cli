module;
#include <concepts>
#include <expected>
#include <filesystem>
#include <memory>
#include <print>
#include <string_view>
export module jowi.crogger:log_emitter;
import :log_error;

namespace fs = std::filesystem;

namespace jowi::crogger {
  export template <typename T>
  concept basic_emitter = requires(const T emitter, std::string_view data) {
    { emitter.emit(data) } -> std::same_as<std::expected<void, log_error>>;
  };

  // Void specialization - abstract base class
  export template <typename T = void> struct log_emitter;

  export template <> struct log_emitter<void> {
    virtual ~log_emitter() = default;

    virtual std::expected<void, log_error> emit(std::string_view data) const = 0;
  };

  // Template for types that satisfy basic_emitter
  export template <basic_emitter emitter_type>
  struct log_emitter<emitter_type> : private emitter_type, public log_emitter<void> {
    using emitter_type::emitter_type; // Inherit constructors

    // Move constructor
    log_emitter(emitter_type &&emitter) : emitter_type(std::move(emitter)) {}

    std::expected<void, log_error> emit(std::string_view data) const override {
      return emitter_type::emit(data);
    }
  };

  export struct stdout_emitter {
    std::expected<void, log_error> emit(std::string_view data) const {
      std::println("{}", data);
      return {};
    }
  };

  export struct file_emitter {
  private:
    constexpr static auto fcloser = [](FILE *f) { fclose(f); };
    using file_ptr_type = std::unique_ptr<FILE, decltype(fcloser)>;
    file_ptr_type __f;
    fs::path __path;

    file_emitter(file_ptr_type f, fs::path p) : __f{std::move(f)}, __path{std::move(p)} {}

  public:
    std::expected<void, log_error> emit(std::string_view v) const {
      auto res = fwrite(v.data(), sizeof(char), v.length(), __f.get());
      if (res != v.length()) {
        return std::unexpected{
          log_error{log_error_type::io_error, "cannot write to file {}", __path.c_str()}
        };
      }
      return {};
    }

    const fs::path &path() const noexcept {
      return __path;
    }

    static std::expected<file_emitter, log_error> open(const fs::path &p, bool append) {
      FILE *f = fopen(p.c_str(), append ? "a" : "w");
      if (f == nullptr) {
        return std::unexpected{
          log_error{log_error_type::io_error, "cannot open file {}", p.c_str()}
        };
      }
      file_ptr_type ptr{f, fcloser};
      return file_emitter{std::move(ptr), p};
    }
  };
}