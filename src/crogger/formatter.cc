module;
#include <concepts>
#include <expected>
#include <format>
#include <iterator>
export module jowi.crogger:formatter;
import jowi.cli.ui;
import :error;
import :context;
import :emitter;

namespace ui = jowi::cli::ui;
namespace jowi::crogger {
  export template <class T>
  concept is_formatter = requires(
    const T formatter, const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
  ) {
    { formatter.format(ctx, it) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <typename T>
  concept format_checkable = requires(const T formatter, const context &ctx) {
    { formatter.check(ctx) } -> std::same_as<std::expected<void, log_error>>;
  };

  export template <class T = void> struct formatter;

  export template <> struct formatter<void> {
    virtual ~formatter() = default;

    virtual std::expected<void, log_error> check(const context &) const = 0;
    virtual std::expected<void, log_error> format(
      const context &, std::back_insert_iterator<stream_emitter<void>> &
    ) const = 0;
  };
  export template <is_formatter formatter_type>
  struct formatter<formatter_type> : private formatter_type, public formatter<void> {
    using formatter_type::formatter_type; // Inherit constructors

    // Move constructor
    formatter(formatter_type &&formatter) : formatter_type(std::move(formatter)) {}

    std::expected<void, log_error> format(
      const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
    ) const override {
      return formatter_type::format(ctx, it);
    }

    std::expected<void, log_error> check(const context &ctx) const override {
      if constexpr (format_checkable<formatter_type>) {
        return formatter_type::check(ctx);
      } else {
        return {}; // Return success (void) as default
      }
    }
  };

  export struct colorful_formatter {
    ui::text_format get_level_color(unsigned int lvl) const noexcept {
      if (lvl < 10) return ui::text_format{}.fg(ui::color::cyan());
      else if (lvl < 20)
        return ui::text_format{}.fg(ui::color::blue());
      else if (lvl < 30)
        return ui::text_format{}.fg(ui::color::green());
      else if (lvl < 40)
        return ui::text_format{}.fg(ui::color::yellow());
      else if (lvl < 50)
        return ui::text_format{}.fg(ui::color::magenta());
      else
        return ui::text_format{}.fg(ui::color::red());
    }
    std::expected<void, log_error> format(
      const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
    ) const {
      std::format_to(
        it,
        "{}[{}]{} {:%FT%TZ} ",
        ui::cli_node::format_begin(get_level_color(ctx.status.level)),
        ctx.status.name,
        ui::cli_node::format_end(),
        ctx.time
      );
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct bw_formatter {
    std::expected<void, log_error> format(
      const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
    ) const {
      std::format_to(it, "[{}] {:%FT%TZ} ", ctx.status.name, ctx.time);
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  export struct empty_formatter {
    std::expected<void, log_error> format(
      const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
    ) const {
      return {};
    }
  };

  export struct plain_formatter {
    std::expected<void, log_error> format(
      const context &ctx, std::back_insert_iterator<stream_emitter<void>> &it
    ) const {
      ctx.message.format(it);
      it = '\n';
      return {};
    }
  };

  template struct formatter<bw_formatter>;
  template struct formatter<colorful_formatter>;
  template struct formatter<empty_formatter>;
};