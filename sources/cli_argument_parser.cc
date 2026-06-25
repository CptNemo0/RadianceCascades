#include "cli_argument_parser.h"

#include <cassert>
#include <format>
#include <print>
#include <stdexcept>
#include <string_view>

#include "aliasing.h"
#include "trie.h"

namespace rc {

namespace {

constexpr std::string_view help_string = R"(
Example usage: .\RadianceCascades.exe --argument --OtherArgument="value"
Argument names are case insensitive.
--help    Displays this message
)";

bool EmptyFunc([[maybe_unused]] std::string_view) {
  return true;
}

void PrintHelp([[maybe_unused]] std::string_view) {
  std::println("{}", help_string);
}

}  // namespace

CliArgumentParser::CliArgumentParser() {
  Init();
}

void CliArgumentParser::Init() {
  using RequiresValue = bool;
  using ValidationExecutionFunctionsPointer =
      ArgumentTrie::ValidationExecutionPair*;

  function_pairs[0].Validate = EmptyFunc;
  function_pairs[0].Execute = PrintHelp;

  trie.InsertArgument("help", RequiresValue{false},
                      ValidationExecutionFunctionsPointer{&function_pairs[0]});
  trie.InsertArgument("template", RequiresValue{true},
                      ValidationExecutionFunctionsPointer{&function_pairs[0]});
}

std::string_view CliArgumentParser::GetHelp() const {
  return help_string;
}

void CliArgumentParser::Parse(const int argc, char* argv[]) {
  for (int i{1}; i < argc; ++i) {
    ParseArgument(argv[i]);
  }
};

void CliArgumentParser::ParseArgument(char* argv) {
  using Node = ArgumentTrie::Node;

  if (argv[0] != '-' || argv[1] != '-') {
    throw std::runtime_error(
        std::format("[Cli Argument Parser ERROR]: While parsing argument: "
                    "'{}'. Arguments should start with a "
                    "double dash '--'!\n{}",
                    argv, GetHelp()));
  }

  const Node* node = trie.GetIterator();

  u64 i{2};
  for (; argv[i] != '=' && argv[i] != 0; ++i) {
    if (Node* next = node->AdvanceTo(argv[i]); next) {
      node = next;
    } else {
      throw std::runtime_error(std::format(
          "[Cli Argument Parser ERROR]: Invalid argument!\n{}", GetHelp()));
    }
  }

  if (!node->end_of_word) {
    throw std::runtime_error(
        std::format("[Cli Argument Parser ERROR]: Parsed string '{}' is not a "
                    "valid argument!\n{}",
                    argv + 2, GetHelp()));
  }

  const std::string_view argument_name{argv + 2, i - 2};

  if (argv[i] == '=') {
    if (!node->requires_value) {
      throw std::runtime_error(
          std::format("[Cli Argument Parser ERROR]: Argument '{}' does not "
                      "need a value!\n{}",
                      argument_name, GetHelp()));
    }

    const std::string_view value{(argv + i + 1)};

    if (value.empty()) {
      throw std::runtime_error(
          std::format("[Cli Argument Parser ERROR]: No value supplied for the "
                      "'{}' argument!\n{}",
                      argument_name, GetHelp()));
    }

    if (node->functions && node->functions->Validate &&
        node->functions->Execute && node->functions->Validate(value)) {
      node->functions->Execute(value);
      return;
    }
  }

  if (argv[i] == 0) {
    if (node->requires_value) {
      throw std::runtime_error(std::format(
          "[Cli Argument Parser ERROR]: Argument '{}' needs a value!\n{}",
          argument_name, GetHelp()));
    }

    if (node->functions && node->functions->Execute) {
      node->functions->Execute("");
    }
  }
}

}  // namespace rc
