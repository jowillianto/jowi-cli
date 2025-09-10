module;
#include <chrono>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <print>
#include <source_location>
#include <string>
export module jowi.crogger:logger;
import jowi.cli.ui;
import :log_error;
import :log_metadata;

namespace ui = jowi::cli::ui;
namespace fs = std::filesystem;
namespace jowi::crogger {

  /**
   * log_formatter: A base class for polymorphic log formatters.
   * This class provides a common interface for formatters to be composed
   * together to generate log formatting based on the input data.
   */
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

  struct log_status_banner : public log_formatter {
  private:
    std::array<ui::text_format, 6> __fmts;

  public:
    log_status_banner(
      std::array<ui::text_format, 6> fmts = {
        ui::text_format{}.fg(ui::color::cyan()),
        ui::text_format{}.fg(ui::color::blue()),
        ui::text_format{}.fg(ui::color::green()),
        ui::text_format{}.fg(ui::color::yellow()),
        ui::text_format{}.fg(ui::color::magenta()),
        ui::text_format{}.fg(ui::color::red())
      }
    ) : __fmts{std::move(fmts)} {}
    log_status_banner &operator=(std::initializer_list<ui::text_format> l) {
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
        ui::cli_nodes{
          ui::cli_node::format_begin(__fmts[fmt_id]),
          ui::cli_node::text("[{}]", data.status.name),
          ui::cli_node::format_end()
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