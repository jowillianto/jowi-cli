# Moderna CLI
A C++23 based command line application builder. Currently, the following library is best suited for the writing of command line application that operate based on the arguments given to it. 

## Making a simple application
```cpp
import jowi.cli;
import jowi.tui;
namespace cli = jowi::cli;
namespace tui = jowi::tui;
Cli::AppIdentity app_id{
  .name = "My First App",
  .description = "I made this with jowi-cli",
  .author = "Me",
  .license = "MIT License",
  .version = Cli::AppVersion{0, 0, 0}
};

int main(int argc, const char** argv) {
  auto app = Cli::App{app_id, argc, argv};
  app.add_argument("--hello-world").help("Says Hello World").required();
  app.parse_args();
}
```
