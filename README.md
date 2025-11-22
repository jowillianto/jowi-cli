# jowi::cli

A C++23 command-line toolkit built on standard-library facilities plus the in-repo `jowi.generic` utilities. Modules:
```cpp
- jowi.cli      (core CLI builder)
- jowi.tui      (terminal UI primitives)
- jowi.crogger  (logging)
```
No other runtime dependencies are required.

## Quick Start (Hello World)

```cpp
import jowi.cli;
import jowi.tui;
namespace cli = jowi::cli;
namespace tui = jowi::tui;

cli::AppIdentity id{
  .name = "hello",
  .description = "Minimal example",
  .author = "You",
  .license = "MIT",
  .version = cli::AppVersion{0, 1, 0}
};

int main(int argc, const char** argv) {
  auto app = cli::App{id, argc, argv};
  app.add_argument("--message").help("What to print").optional();
  app.parse_args().value(); // or cli::App::expect(...)

  auto msg = app.args().first_of("--message").value_or("Hello, world!");
  auto dom = tui::DomNode::vstack(
    tui::Layout{}
      .append_child(tui::Paragraph("Hello, {}!", msg).no_newline()) // formatted, no newline
      .append_child(tui::Paragraph(" <- rendered with tui::Paragraph"))
  );
  tui::out_terminal.render(dom);
}
```

## Terminal UI (module `jowi.tui`)

Every exported class and utility lives under `jowi::tui` and focuses on building simple styled DOM trees that render to ANSI terminals.

- **Paragraph & Layout first** – `Paragraph` is the text leaf; it supports `std::format` construction and `no_newline()` to keep the cursor on the same line. `Layout` is a container of children.
```cpp
auto p = tui::Paragraph("Row {}", 1).no_newline();
auto layout = tui::Layout{}.append_child(std::move(p));
```
- **Styling layouts** – Use `DomStyle` to indent and color a layout; chain `indent()`, `bg()`, `fg()`, `effect()`, or `effects(range)`.
```cpp
auto style = tui::DomStyle{}.indent(2).fg(tui::RgbColor::green());
auto styled = tui::Layout{}.style(style).append_child(tui::Paragraph("Indented text"));
```
- **Building DomNode trees** – `DomNode::paragraph(...)` and `DomNode::vstack(Layout)` enable a builder pattern: chain `append_child` on `Layout` to grow the tree, then wrap it.
```cpp
auto dom = tui::DomNode::vstack(
  tui::Layout{}
    .style(style)
    .append_child(tui::Paragraph("Title"))
    .append_child(tui::DomNode::vstack(
      tui::Layout{}.style(tui::DomStyle{}.indent(2))
        .append_child(tui::Paragraph("Line A"))
        .append_child(tui::Paragraph("Line B"))
    ))
);
```
- **Colors and effects** – `RgbColor` provides helpers like `RgbColor::red()` / `bright_blue()`. `TextEffect` covers bold, underline, blink, reverse, etc.
```cpp
auto error_color = tui::RgbColor::bright_red();
tui::DomStyle{}.effect(tui::TextEffect::UNDERLINE);
```
- **RenderError** – exception type used by renderers; construct via `RenderError::format("msg {}", x)` when producing failures.
- **IsDomRenderer** – concept for renderers exposing `render(const DomNode&)` and `clear()`, both returning `std::expected<void, RenderError>`.
- **FileCloser / FileHandle** – small RAII helpers used by the ANSI terminal implementation; you can wrap your own `FILE*` if needed.
```cpp
tui::FileHandle fh{fopen("out.txt","w"), tui::FileCloser{}};
```
- **AnsiFormatter<Iterator>** – static renderer that walks a DOM tree and writes ANSI-styled text into an iterator (e.g., `std::back_insert_iterator<std::string>`).
```cpp
std::string buf;
auto it = std::back_inserter(buf);
tui::AnsiFormatter<std::decay_t<decltype(it)>>::render(dom, it, 0, std::nullopt);
```
- **StringRenderer** – convenience renderer that stores the formatted string internally; call `render(dom)`, `read()` to grab the buffer, and `clear()` to reset.
```cpp
tui::StringRenderer sr;
sr.render(dom);
auto text = sr.read();
```
- **AnsiTerminal** – renders to a `FILE*` with `render(dom)` and clears the screen with `clear()`. Static factories: `stdout_terminal()` / `stderr_terminal()`.
```cpp
tui::AnsiTerminal::stdout_terminal().render(dom);
```
- **out_terminal / err_terminal** – inline singletons bound to stdout/stderr so you can `tui::out_terminal.render(dom);`.

## Logging (module `jowi.crogger`)

`jowi.crogger` offers a minimal logging pipeline: build a message, pass it through a logger, and emit to stdout/stderr or files. A `Logger` is composed of:
- a formatter: shapes the log line and colors/styling (implementations: `ColorfulFormatter`, `BwFormatter`, `PlainFormatter`, `EmptyFormatter`);
- a filter: decides whether to log (`LevelFilter` with comparison helpers, or `NoFilter`);
- an emitter: writes bytes to a destination (`StdoutEmitter`, `StderrEmitter`, `EmptyEmitter`, `FileEmitter::open(...)`), usually wrapped by `StreamEmitter<SsEmitter>` buffers.
- **Messages & levels** – Wrap text with `Message` (inherits `RawMessage`) so formatting is deferred. Use `LogLevel::trace/debug/info/warn/error/critical()` to tag severity.
```cpp
crogger::Message msg{"User {} logged in", user};
```
- **LogError / LogErrorType** – errors surfaced by formatters or emitters; create with `LogError::format_error(...)` or `LogError::io_error(...)`.
- **Formatters** – Pick how logs look. `ColorfulFormatter` uses `jowi.tui` colors, `BwFormatter` prints plain text, `PlainFormatter` writes only the message body, `EmptyFormatter` drops output.
```cpp
crogger::Logger l;
l.set_formatter(crogger::BwFormatter{});
```
- **Filters** – Gate logs by level using `LevelFilter::{equal_to,less_than,greater_than_or_equal_to}`; `NoFilter` passes everything.
```cpp
l.set_filter(crogger::LevelFilter::greater_than_or_equal_to(crogger::LogLevel::info().level));
```
- **Emitters** – Send bytes to stdout/stderr, nothingness, or a file.
```cpp
auto file = crogger::FileEmitter::open("app.log", true).value();
crogger::Logger custom;
custom.set_emitter(std::move(file));
```
- **Logger usage** – Configure formatter/filter/emitter, then log via `crogger::log(logger, level, message)`.
```cpp
crogger::Logger l;
l.set_filter(crogger::LevelFilter::greater_than_or_equal_to(30))
 .set_formatter(crogger::ColorfulFormatter{})
 .set_emitter(crogger::StdoutEmitter{});
crogger::log(l, crogger::LogLevel::warn(), crogger::Message{"Low disk: {}%", 12});
```

### Root logger
`crogger::root()` returns a process-wide logger. Helper functions `trace/debug/info/warn/error/critical` accept either a `Logger` or default to the root logger; customize the root once and reuse it everywhere.
```cpp
using namespace jowi::crogger;
root().set_formatter(ColorfulFormatter{});
info(Message{"Starting {}", id.name});    // uses root()
error(root(), Message{"Failure: {}", reason}); // explicit logger target
```

## CLI (module `jowi.cli`)

Build apps, parse arguments, and wire behaviors.

- **AppIdentity & AppVersion** – Metadata attached to an `App`. Parse versions via `AppVersion::from_string("1.2.3")`.
```cpp
auto ver = cli::AppVersion::from_string("2.0.1").value();
```
- **ArgKey** – Safely represent `-k`/`--key` tokens; build with `ArgKey::make("--out")` or parse via `ArgKey::parse_arg("--out=file")`.
```cpp
auto [key, val] = cli::ArgKey::parse_arg("--name=john").value();
```
- **Arg & built-in validators** – Chain helpers: `required()`, `optional()`, `as_flag()`, `require_value()`, `n_at_least(...)`, `with_default(...)`, or attach `ArgOptionsValidator`, `ArgCountValidator`, `ArgEmptyValidator`, `ArgDefaultValidator`.
```cpp
app.add_argument("--mode", cli::Arg{}.require_value()
  .add_validator(cli::ArgOptionsValidator{}.add_option("fast").add_option("safe")));
```
- **Custom validators** – Implement any subset of `id()`, `help()`, `validate(value)`, and `post_validate(key, args)`; wrap with `add_validator`.
```cpp
struct PositiveInt {
  std::optional<std::string> id() const { return "positive_int"; }
  std::expected<void, cli::ParseError> validate(std::optional<std::string_view> v) const {
    auto num = cli::parse_arg<int>(*v);
    if (!num || num.value() <= 0) return std::unexpected{cli::ParseError::invalid_value("must be > 0")};
    return {};
  }
};
app.add_argument("--count").add_validator(PositiveInt{});
```
- **ArgParser / ParsedArg** – `ArgParser::parse` walks `RawArgs` and fills `ParsedArg`, which exposes `arg()` (positional), `first_of(key)`, `contains(key)`, `count(key)`, and iteration over captured pairs.
```cpp
if (app.args().contains("--verbose")) { /* ... */ }
```
- **App** – Central type that wires the parser, help output, and error handling. Use `add_argument(...)`, `parse_args(auto_help=true, auto_exit=true)`, and helpers `App::expect(...)`, `App::expect_or(...)`, `App::error(...)`. `help_dom()` renders structured help using the TUI stack.
```cpp
app.add_argument("-v").as_flag().help("Verbose");
cli::App::expect(app.parse_args(), "Failed: {}");
```
- **ActionBuilder** – Build subcommands/actions for a positionally-chosen verb and enforce allowed options automatically.
```cpp
cli::ActionBuilder builder{app, "Choose an action"};
builder.add_action("serve", "Start server", [](cli::App& app){ /* ... */ })
       .add_action("migrate", "Run migrations", [](cli::App& app){ /* ... */ })
       .update_args() // sync help/options
       .run();        // parses args and dispatches
```
- **Shortcut parsing** – `parse_arg<T>(std::string_view)` is implemented for integers, floats, `std::string`, `std::filesystem::path`, and `cli::AppVersion`, returning `std::expected<T, ParseError>`.
```cpp
auto port = cli::parse_arg<int>("8080").value();
```

With these pieces you can compose styled terminal output, structured logging, and ergonomic argument parsing within a single module-first C++23 codebase.
