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
  template <class T> using Ref = std::reference_wrapper<T>;

  struct ArgDecl {
    Arg pos;
    generic::key_vector<ArgKey, Arg> params;

    ArgDecl(Arg pos) : pos{std::move(pos)}, params{} {}

    std::expected<Ref<ParsedArg>, ParseError> parse(
      uint64_t pos_id, ParsedArg &args, bool parse_positional
    ) const {
      auto opt_current_arg = args.raw();
      if (parse_positional && opt_current_arg.has_value()) {
        std::string_view current_arg = opt_current_arg.value();
        if (ArgKey::is_arg_key(current_arg)) {
          return std::unexpected{ParseError{
            ParseErrorType::INVALID_VALUE,
            "arg{} - expected positional found keyword argument",
            pos_id
          }};
        }
        auto res = pos.validate(current_arg);
        if (!res) {
          return std::unexpected{
            ParseError{res.error().err_type(), "arg{} - {}", pos_id, res.error().msg_only()}
          };
        }
        args.add_argument(current_arg);
        auto post_res = pos.post_validate(std::nullopt, args);
        opt_current_arg = args.next_raw();
      }
      bool is_parser_run = true;
      while (is_parser_run && opt_current_arg.has_value()) {
        is_parser_run = false;
        auto kv_res = ArgKey::parse_arg(opt_current_arg.value());
        if (!kv_res) break;
        auto kv = std::move(kv_res.value());
        for (const auto &[k, arg] : params) {
          if (kv.first == k) {
            is_parser_run = true;
            auto res = arg.validate(kv.second)
                         .or_else([&](auto &&e) {
                           if (e.err_type() == ParseErrorType::NO_VALUE_GIVEN) {
                             kv.second = args.next_raw();
                             return arg.validate(kv.second);
                           } else {
                             return std::expected<void, ParseError>{std::unexpected{std::move(e)}};
                           }
                         })
                         .transform_error([&](auto &&e) {
                           return ParseError{e.err_type(), "{} - {}", k, e.msg_only()};
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
          return std::unexpected{
            ParseError{res.error().err_type(), "{} - {}", k, res.error().msg_only()}
          };
        }
      }
      return std::ref(args);
    }

    uint64_t param_size() const noexcept {
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

  export struct ArgParser {
  private:
    std::vector<ArgDecl> __args;

  public:
    ArgParser() : __args{} {
      add_argument().help("main executable");
    }
    Arg &add_argument(ArgKey k, Arg arg = Arg::flag()) {
      return __args.back().params.emplace(k, std::move(arg));
    }
    Arg &add_argument(std::string_view k, Arg arg = Arg::flag()) {
      return __args.back().params.emplace(ArgKey::make(k).value(), std::move(arg));
    }
    Arg &add_argument() {
      __args.emplace_back(Arg::positional());
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
    std::expected<Ref<ParsedArg>, ParseError> parse(ParsedArg &args) const {
      for (auto arg_id = std::max(0, static_cast<int>(args.size()) - 1); arg_id != size();
           arg_id += 1) {
        auto res = __args[arg_id].parse(arg_id, args, args.size() <= arg_id);
        if (!res) {
          return std::unexpected{std::move(res.error())};
        }
      }
      return std::ref(args);
    }
    std::expected<ParsedArg, ParseError> parse(RawArgs raw) const {
      ParsedArg args{raw};
      return parse(args).transform([](auto &&args) { return std::move(args); });
    }
  };
}
