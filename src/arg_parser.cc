module;
#include <expected>
#include <format>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>
export module jowi.cli:arg_parser;
import jowi.generic;
import :arg_key;
import :parsed_arg;
import :arg;
import :parse_error;

namespace jowi::cli {
  template <class T> using ref = std::reference_wrapper<T>;

  struct arg_decl {
    arg pos;
    generic::key_vector<arg_key, arg> params;

    arg_decl(arg pos) : pos{std::move(pos)}, params{} {}

    std::expected<ref<parsed_arg>, parse_error> parse(
      size_t pos_id, parsed_arg &args, bool parse_positional
    ) const {
      auto opt_current_arg = args.raw();
      if (parse_positional && opt_current_arg.has_value()) {
        std::string_view current_arg = opt_current_arg.value();
        if (arg_key::is_arg_key(current_arg)) {
          return std::unexpected{parse_error{parse_error_type::INVALID_VALUE, "arg{}", pos_id}};
        }
        auto res = pos.validate(current_arg);
        if (!res) {
          return std::unexpected{res.error().reformat("{2:}: {0:} {1:}", pos_id)};
        }
        args.add_argument(current_arg);
        auto post_res = pos.post_validate(std::nullopt, args);
        opt_current_arg = args.next_raw();
      }
      bool is_parser_run = true;
      while (is_parser_run && opt_current_arg.has_value()) {
        is_parser_run = false;
        auto kv_res = arg_key::parse_arg(opt_current_arg.value());
        if (!kv_res) break;
        auto kv = std::move(kv_res.value());
        for (const auto &[k, arg] : params) {
          if (kv.first == k) {
            is_parser_run = true;
            auto res = arg.validate(kv.second)
                         .or_else([&](auto &&e) {
                           if (e.err_type() == parse_error_type::NO_VALUE_GIVEN) {
                             kv.second = args.next_raw();
                             return arg.validate(kv.second);
                           } else {
                             return std::expected<void, parse_error>{std::unexpected{e}};
                           }
                         })
                         .transform_error([&](auto &&e) {
                           return std::move(e.reformat("{2:}: {0:} {1:}", k));
                         });
            if (!res) {
              return std::unexpected{std::move(res.error())};
            }
            args.add_argument(kv.first, kv.second.value_or(""));
            opt_current_arg = args.next_raw();
          }
        }
        if (!is_parser_run) {
          break;
        }
      }
      for (const auto &[k, arg] : params) {
        auto res = arg.post_validate(std::cref(k), args);
        if (!res) {
          return std::unexpected{std::move(res.error().reformat("{2:}: {0:} {1:}", k))};
        }
      }
      return std::ref(args);
    }

    size_t param_size() const noexcept {
      return params.size();
    }

    bool empty() const noexcept {
      return params.empty();
    }

    auto begin() const noexcept {
      return params.begin();
    }
    auto end() const noexcept {
      return params.end();
    }
  };

  export struct arg_parser {
  private:
    std::vector<arg_decl> __args;

  public:
    arg_parser() : __args{} {
      add_argument().help("main executable");
    }
    arg &add_argument(arg_key k, arg arg = arg::flag()) {
      return __args.back().params.emplace(k, std::move(arg));
    }
    arg &add_argument(std::string_view k, arg arg = arg::flag()) {
      return __args.back().params.emplace(arg_key::make(k).value(), std::move(arg));
    }
    arg &add_argument() {
      __args.emplace_back(arg::positional());
      return __args.back().pos;
    }

    auto begin() const noexcept {
      return __args.begin();
    }
    auto end() const noexcept {
      return __args.end();
    }
    auto size() const noexcept {
      return __args.size();
    }
    auto param_beg() const noexcept {
      return __args.back().params.begin();
    }
    auto param_end() const noexcept {
      return __args.back().params.end();
    }
    auto param_size() const noexcept {
      return __args.back().params.size();
    }

    /*
      Argument Parsing
    */
    std::expected<ref<parsed_arg>, parse_error> parse(parsed_arg &args) const {
      for (auto arg_id = std::max(0, static_cast<int>(args.size()) - 1); arg_id != size();
           arg_id += 1) {
        auto res = __args[arg_id].parse(arg_id, args, args.size() <= arg_id);
        if (!res) {
          return std::unexpected{std::move(res.error())};
        }
      }
      return std::ref(args);
    }
    std::expected<parsed_arg, parse_error> parse(raw_args raw) const {
      parsed_arg args{raw};
      return parse(args).transform([](auto &&args) { return std::move(args); });
    }
  };
}