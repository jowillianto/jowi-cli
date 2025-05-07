#include <pybind11/attr.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <expected>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
import moderna.cli;
import moderna.generic;

namespace py = pybind11;
namespace cli = moderna::cli;
namespace generic = moderna::generic;

template <class T> struct decl {};

namespace PYBIND11_NAMESPACE {
  namespace detail {
    template <class T, generic::is_whatable_error E> struct type_caster<std::expected<T, E>> {
    public:
      /**
       * This macro establishes the name 'inty' in
       * function signatures and declares a local variable
       * 'value' of type inty
       */
      using expected_t = std::expected<T, E>;
      using variant_t = std::variant<T, std::runtime_error>;
      PYBIND11_TYPE_CASTER(expected_t, const_name("expected"));
      static handle cast(expected_t src, return_value_policy policy, handle parent) {
        if (src)
          return pybind11::detail::make_caster<variant_t>::cast(
            variant_t{std::move(src.value())}, policy, parent
          );
        else
          return pybind11::detail::make_caster<variant_t>::cast(
            variant_t{std::runtime_error{src.error().what()}}, policy, parent
          );
      }
    };
  }
}

template <class T, std::invocable<const T &> F = std::identity> struct string_maker {
  F proj;
  string_maker(F &&f, decl<T> v) : proj{std::forward<F>(f)} {}
  string_maker(decl<T> v)
    requires(std::is_default_constructible_v<F>)
  {}

  std::string operator()(const T &v) const {
    return std::format("{}", std::invoke(proj, v));
  }
};

template <class T> struct equalizer {
  bool operator()(const T &l, const T &r) {
    return l == r;
  }
};
template <class T> struct ltizer {
  bool include_eq = false;
  bool operator()(const T &l, const T &r) {
    if (include_eq) {
      return l < r || l == r;
    } else {
      return l < r;
    }
  }
};
template <class T> struct gtizer {
  bool include_eq = false;
  bool operator()(const T &l, const T &r) {
    if (include_eq) {
      return l > r || l == r;
    } else {
      return l > r;
    }
  }
};

PYBIND11_MODULE(moderna_cli, m) {
  py::class_<cli::argument_value>(m, "ArgumentValue")
    .def("cast_int", &cli::argument_value::cast<int>)
    .def("cast_str", &cli::argument_value::cast<std::string_view>)
    .def("cast_app_version", &cli::argument_value::cast<cli::app_version>)
    .def("cast_float", &cli::argument_value::cast<float>);
  py::class_<cli::app_version>(m, "AppVersion")
    .def(py::init<int, int, int>())
    .def("__eq__", equalizer<cli::app_version>{}, py::is_operator())
    .def("__lt__", ltizer<cli::app_version>{}, py::is_operator())
    .def("__gt__", gtizer<cli::app_version>{}, py::is_operator())
    .def("__str__", string_maker{decl<cli::app_version>{}})
    .def("__repr__", string_maker{decl<cli::app_version>{}})
    .def_readwrite("major", &cli::app_version::major)
    .def_readwrite("minor", &cli::app_version::minor)
    .def_readwrite("patch", &cli::app_version::patch)
    .def_static("from_string", &cli::app_version::from_string);
  py::class_<cli::app_identity>(m, "AppIdentity")
    .def(
      py::init<
        std::string,
        std::string,
        std::optional<std::string>,
        std::optional<std::string>,
        cli::app_version>()
    )
    .def_readwrite("name", &cli::app_identity::name)
    .def_readwrite("description", &cli::app_identity::description)
    .def_readwrite("author", &cli::app_identity::author)
    .def_readwrite("license", &cli::app_identity::license)
    .def_readwrite("version", &cli::app_identity::version)
    .def("__str__", string_maker{decl<cli::app_version>{}})
    .def("__repr__", string_maker{decl<cli::app_version>{}});
  py::class_<cli::parameter_argument>(m, "ParameterArgument")
    .def("required", &cli::parameter_argument::required)
    .def("optional", &cli::parameter_argument::optional)
    .def("multiple", &cli::parameter_argument::multiple)
    .def("singular", &cli::parameter_argument::singular)
    .def("as_flag", &cli::parameter_argument::as_flag)
    .def("help", &cli::parameter_argument::help)
    .def_readonly("help_text", &cli::parameter_argument::help_text)
    .def_readonly("is_required", &cli::parameter_argument::is_required)
    .def_readonly("require_value", &cli::parameter_argument::require_value)
    .def_readonly("max_size", &cli::parameter_argument::max_size);
  py::class_<cli::app_position_argument>(m, "PositionArgument")
    .def("help", &cli::app_position_argument::help)
    .def_readonly("help_text", &cli::app_position_argument::help_text);
  py::class_<cli::parsed_argument>(m, "ParsedArgument")
    .def("current_positional", &cli::parsed_argument::current_positional)
    .def("first_of", &cli::parsed_argument::first_of<std::string_view>)
    .def("contains", &cli::parsed_argument::contains<std::string_view>)
    .def("count", &cli::parsed_argument::count<std::string_view>)
    .def("filter", [](const cli::parsed_argument &parsed_args, std::string_view key) {
      auto key_filtered_view = parsed_args.filter(key);
      return std::vector<std::string_view>{key_filtered_view.begin(), key_filtered_view.end()};
    });
  py::class_<cli::app_action_builder>(m, "AppActionBuilder")
    .def(
      "add_action", &cli::app_action_builder::add_action<const std::function<void(cli::app &)> &>
    )
    .def("run", &cli::app_action_builder::run);
  py::class_<cli::app>(m, "App")
    .def(py::init<cli::app_identity, int, const char **, bool>())
    .def(
      "add_argument",
      [](cli::app &app, std::string_view p) -> cli::parameter_argument & {
        return app.add_argument(p);
      }
    )
    .def("add_argument", &cli::app::add_argument<bool>)
    .def("add_help_argument", &cli::app::add_help_argument)
    .def("arguments", &cli::app::arguments)
    .def("argument_parser", &cli::app::argument_parser)
    .def("name", &cli::app::name)
    .def("description", &cli::app::description)
    .def("version", &cli::app::description)
    .def("id", &cli::app::id)
    .def("parse_argument", &cli::app::parse_argument)
    .def("build_action", &cli::app::build_action);
}