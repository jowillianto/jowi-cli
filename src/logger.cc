module;
#include <chrono>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <print>
#include <source_location>
#include <string>
export module moderna.cli:logger;
export import :terminal_nodes;
namespace moderna::cli {
  namespace fs = std::filesystem;
  enum struct log_error_type {
    format_error,
    io_error,
  };
}
template <typename CharT> struct std::formatter<moderna::cli::log_error_type, CharT> {
  constexpr auto parse(auto &ctx) {
    return ctx.begin();
  }

  auto format(moderna::cli::log_error_type type, auto &ctx) const {
    switch (type) {
      case moderna::cli::log_error_type::format_error:
        return std::format_to(ctx.out(), "format_error");
      case moderna::cli::log_error_type::io_error:
        return std::format_to(ctx.out(), "io_error");
    }
  }
};
namespace moderna::cli {
  struct log_error : public std::exception {
  private:
    log_error_type __type;
    std::string __message;

  public:
    template <typename... Args>
      requires(std::formattable<Args, char> && ...)
    log_error(log_error_type t, std::format_string<Args...> fmt, Args &&...args) : __type(t) {
      auto back_inserter = std::back_inserter(__message);
      std::format_to(back_inserter, "{}: ", t);
      std::format_to(back_inserter, fmt, std::forward<Args>(args)...);
    }

    const char *what() const noexcept override {
      return __message.c_str();
    }
  };
  struct log_status {
    std::string name;
    unsigned int level;

    static log_status debug() {
      return {"DEBUG", 0};
    }

    static log_status info() {
      return {"INFO", 10};
    }

    static log_status warn() {
      return {"WARN", 20};
    }

    static log_status critical() {
      return {"CRITICAL", 30};
    }

    static log_status error() {
      return {"ERROR", 40};
    }
  };

  struct log_metadata {
    log_status status;
    std::string message;
    std::source_location loc;
    std::chrono::system_clock::time_point time;

    static log_metadata test(std::source_location loc = std::source_location::current()) {}
  };

  struct log_formatter {
    virtual std::expected<void, log_error> check(const log_metadata &test_data) const = 0;
    virtual void format(
      const log_metadata &data, std::back_insert_iterator<std::string> &buf
    ) const = 0;
    virtual ~log_formatter() = default;
  };

  struct log_filter {
    virtual bool filter(const log_metadata &data) const = 0;
    virtual ~log_filter() = default;
  };

  struct log_emitter {
    virtual std::expected<void, log_error> emit(std::string_view v) const = 0;
    virtual ~log_emitter() = default;
  };

  struct log_formatter_chain : public log_formatter {
  private:
    std::vector<std::unique_ptr<log_formatter>> __fmts;

  public:
    template <class... Args>
      requires(std::derived_from<log_formatter, Args> && ...)
    log_formatter_chain(Args &&...args) : __fmts{} {
      add_formatters(std::forward<Args>(args)...);
    }

    template <class... Args>
      requires(std::derived_from<log_formatter, Args> && ...)
    log_formatter_chain &add_formatters(Args &&...args) {
      (__fmts.emplace_back(std::forward<Args>(args)), ...);
      return *this;
    }

    size_t size() const noexcept {
      return __fmts.size();
    }
    bool empty() const noexcept {
      return __fmts.empty();
    }
    constexpr auto begin() const noexcept {
      return __fmts.begin();
    }
    constexpr auto end() const noexcept {
      return __fmts.end();
    }

    /*
      log_formatter impl
    */
    std::expected<void, log_error> check(const log_metadata &test_data) const override {
      for (const auto &fmt : __fmts) {
        auto res = fmt->check(test_data);
        if (!res) {
          return std::unexpected{std::move(res.error())};
        }
      }
      return {};
    }
    void format(
      const log_metadata &data, std::back_insert_iterator<std::string> &buf
    ) const override {
      std::ranges::for_each(__fmts, [&](const auto &fmt) { fmt->format(data, buf); });
    }
  };

  struct log_emitter_chain : public log_emitter {
  private:
    std::vector<std::unique_ptr<log_emitter>> __emts;

  public:
    template <class... Args>
      requires(std::derived_from<log_emitter, Args> && ...)
    log_emitter_chain(Args &&...args) : __emts{} {}

    template <class... Args>
      requires(std::derived_from<log_emitter, Args> && ...)
    log_emitter_chain &add_emitter(Args &&...args) {
      (__emts.emplace_back(std::forward<Args>(args)), ...);
      return *this;
    }

    size_t size() const noexcept {
      return __emts.size();
    }
    bool empty() const noexcept {
      return __emts.empty();
    }
    constexpr auto begin() const noexcept {
      return __emts.begin();
    }
    constexpr auto end() const noexcept {
      return __emts.end();
    }

    std::expected<void, log_error> emit(std::string_view v) const override {
      for (const auto &e : __emts) {
        auto res = e->emit(v);
        if (!res) {
          return std::unexpected{std::move(res.error())};
        }
      }
      return {};
    }
  };

  struct log_status_banner : public log_formatter {
  private:
    std::array<text_format, 5> __fmts;

  public:
    log_status_banner(
      std::array<text_format, 5> fmts = {
        text_format{}.fg(color::blue()),
        text_format{}.fg(color::green()),
        text_format{}.fg(color::yellow()),
        text_format{}.fg(color::magenta()),
        text_format{}.fg(color::red())
      }
    ) : __fmts{std::move(fmts)} {}
    log_status_banner &operator=(std::initializer_list<text_format> l) {
      auto max_iter = std::min(__fmts.size(), l.size());
      for (auto i = 0; i < max_iter; i += 1) {
        __fmts[i] = std::move(*(l.begin() + i));
      }
      return *this;
    }

    std::expected<void, log_error> check(const log_metadata &test_data) const override {
      return {};
    }
    void format(
      const log_metadata &data, std::back_insert_iterator<std::string> &buf
    ) const override {
      auto fmt_id = std::max(__fmts.size(), static_cast<size_t>(data.status.level));
      std::format_to(
        buf,
        "{}",
        terminal_nodes{
          terminal_node::format_begin(__fmts[fmt_id]),
          terminal_node::text("[{}]", data.status.name),
          terminal_node::format_end()
        }
      );
    }
  };

  struct stdout_emitter : public log_emitter {
  public:
    stdout_emitter() {}
    std::expected<void, log_error> emit(std::string_view v) const override {
      std::println("{}", v);
      return {};
    }
  };

  struct file_emitter : public log_emitter {
  private:
    constexpr static auto fcloser = [](FILE *f) { fclose(f); };
    using file_ptr_type = std::unique_ptr<FILE, decltype(fcloser)>;
    file_ptr_type __f;
    fs::path __path;

    file_emitter(file_ptr_type f, fs::path p) : __f{std::move(f)}, __path{std::move(p)} {}

  public:
    std::expected<void, log_error> emit(std::string_view v) const override {
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