# Moderna CLI
A C++23 based command line application builder. Currently, the following library is best suited for the writing of command line application that operate based on the arguments given to it. 

## Making a simple application
```cpp
import moderna.cli;
namespace cli = moderna::cli;
cli::app_identity app_id{
  .name = "My First App",
  .description = "I made this with moderna-cli",
  .author = "Me",
  .license = "MIT License",
  .version = cli::app_version{0, 0, 0}
};

int main(int argc, const char** argv) {
  auto app = cli::app{app_id, argc, argv};
  app.add_argument("--hello-world").help("Says Hello World").required();
  app.parse_args();
}
```