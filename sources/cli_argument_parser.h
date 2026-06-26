#ifndef RADIANCE_CASCADES_CLI_ARGUMENT_PARSER_H_
#define RADIANCE_CASCADES_CLI_ARGUMENT_PARSER_H_

#include <array>
#include <memory>
#include <string_view>

#include "app.h"
#include "trie.h"

namespace rc {
class CliArgumentParser {
 public:
  static CliArgumentParser& Instance() {
    static CliArgumentParser instance;
    return instance;
  }

  std::unique_ptr<App::Config> Parse(const int argc, char* argv[]);

  CliArgumentParser(const CliArgumentParser& other) = delete;
  CliArgumentParser(CliArgumentParser&& other) = delete;

  CliArgumentParser& operator=(const CliArgumentParser& other) = delete;
  CliArgumentParser& operator=(CliArgumentParser&& other) = delete;

  ~CliArgumentParser() = default;

 private:
  CliArgumentParser();
  void Init();

  void ParseArgument(char* argv, App::Config* config);

  std::string_view GetHelp() const;

  ArgumentTrie trie;

  std::array<ArgumentTrie::ValidationExecutionPair, 1> function_pairs;
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_CLI_ARGUMENT_PARSER_H_
