module;
#include <chrono>
#include <expected>
#include <filesystem>
#include <format>
#include <iterator>
#include <string>
export module jowi.crogger:log_formatter;
import jowi.cli.ui;
import :log_error;
import :log_metadata;

namespace ui = jowi::cli::ui;
namespace fs = std::filesystem;
namespace jowi::crogger {
  export template <typename T>
  concept basic_formatter = requires(
    const T formatter, const log_metadata &data, std::back_insert_iterator<std::string> &it
  ) {
    { formatter.format(data, it) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <typename T>
  concept checkable_formatter = requires(const T formatter, const log_metadata &data) {
    { formatter.check(data) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <class T = void> struct log_formatter;

  // Void specialization - abstract base class
  export template <> struct log_formatter<void> {
    virtual ~log_formatter() = default;

    virtual std::expected<void, log_error> check(const log_metadata &data) const = 0;
    virtual std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const = 0;
  };

  // Template for types that satisfy basic_formatter
  export template <basic_formatter formatter_type>
  struct log_formatter<formatter_type> : private formatter_type, public log_formatter<void> {
    using formatter_type::formatter_type; // Inherit constructors

    // Move constructor
    log_formatter(formatter_type &&formatter) : formatter_type(std::move(formatter)) {}

    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const override {
      return formatter_type::format(data, it);
    }

    std::expected<void, log_error> check(const log_metadata &data) const override {
      if constexpr (checkable_formatter<formatter_type>) {
        return formatter_type::check(data);
      } else {
        return {}; // Return success (void) as default
      }
    }
  };

  export struct level_formatter {
    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      std::format_to(it, "[{}]", data.status.name);
      return {}; // success
    }
  };

  export struct colorful_level_formatter {
  private:
  private:
    std::array<ui::text_format, 6> __fmts;

  public:
    colorful_level_formatter(
      std::array<ui::text_format, 6> fmts = {
        ui::text_format{}.fg(ui::color::cyan()),
        ui::text_format{}.fg(ui::color::blue()),
        ui::text_format{}.fg(ui::color::green()),
        ui::text_format{}.fg(ui::color::yellow()),
        ui::text_format{}.fg(ui::color::magenta()),
        ui::text_format{}.fg(ui::color::red())
      }
    ) : __fmts{std::move(fmts)} {}
    colorful_level_formatter &operator=(std::initializer_list<ui::text_format> l) {
      auto max_iter = std::min(__fmts.size(), l.size());
      for (auto i = 0; i < max_iter; i += 1) {
        __fmts[i] = std::move(*(l.begin() + i));
      }
      return *this;
    }

    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      auto fmt_id = std::min(__fmts.size() - 1, static_cast<size_t>(data.status.level / 10));
      std::format_to(
        it,
        "{}[{}]{}",
        ui::cli_node::format_begin(__fmts[fmt_id]),
        data.status.name,
        ui::cli_node::format_end()
      );
      return {};
    }
  };

  export struct time_formatter {
    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      std::format_to(it, "[{:%DT%T%z}]", data.time);
      return {};
    }
  };

  export struct file_loc_formatter {
    std::optional<fs::path> work_dir = std::nullopt;
    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      fs::path file_name{data.loc.file_name()};
      if (work_dir) {
        file_name = fs::relative(*work_dir, file_name);
      } else {
        file_name = file_name.parent_path().filename() / file_name.filename();
      }
      std::format_to(it, "[{}:{}]", file_name.c_str(), data.loc.line());
      return {};
    }
  };

  export struct message_formatter {
    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      std::format_to(it, "{}", data.message);
      return {};
    }
  };

  export template <basic_formatter... fmts> struct formatter_tuple {

  private:
    std::tuple<std::decay_t<fmts>...> __formatters;
    char __sep;

    template <size_t i = 0>
    std::expected<void, log_error> __format(
      const log_metadata &data,
      std::back_insert_iterator<std::string> &it,
      std::expected<void, log_error> res = {}
    ) const {
      if constexpr (i == sizeof...(fmts)) {
        return res;
      } else {
        // This will tail call ?
        return __format<i + 1>(data, it, std::move(res).and_then([&]() {
          if constexpr (i != 0) {
            std::format_to(it, "{}", __sep);
          }
          return std::get<i>(__formatters).format(data, it);
        }));
      }
    }

    template <size_t i = 0>
    std::expected<void, log_error> __check(
      const log_metadata &data, std::expected<void, log_error> res = {}
    ) const {
      if constexpr (i == sizeof...(fmts)) {
        return res;
      } else {
        // This will tail call ?
        return __check<i + 1>(data, std::move(res).and_then([&]() {
          if constexpr (checkable_formatter<decltype(std::get<i>(__formatters))>) {
            return std::get<i>(__formatters).check(data);
          } else {
            return std::expected<void, log_error>{};
          }
        }));
      }
    }

  public:
    formatter_tuple(char sep, fmts &&...formatters) :
      __sep{sep}, __formatters(std::forward<fmts>(formatters)...) {}

    std::expected<void, log_error> check(const log_metadata &data) const {
      return __check(data);
    }

    std::expected<void, log_error> format(
      const log_metadata &data, std::back_insert_iterator<std::string> &it
    ) const {
      return __format(data, it);
    }
  };

  export using colorful_formatter = formatter_tuple<
    colorful_level_formatter,
    time_formatter,
    file_loc_formatter,
    message_formatter>;
  export using bw_formatter =
    formatter_tuple<level_formatter, time_formatter, file_loc_formatter, message_formatter>;
}