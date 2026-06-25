#ifndef RADIANCE_CASCADES_CLI_ARGUMENT_PRASER_H_
#define RADIANCE_CASCADES_CLI_ARGUMENT_PRASER_H_

#include <array>
#include <string_view>

#include "trie.h"

namespace rc {
class CliArgumentParser {
 public:
  static CliArgumentParser& Instance() {
    static CliArgumentParser instance;
    return instance;
  }

  void Parse(const int argc, const char* argv[]);

  CliArgumentParser(const CliArgumentParser& other) = delete;
  CliArgumentParser(CliArgumentParser&& other) = delete;

  CliArgumentParser& operator=(const CliArgumentParser& other) = delete;
  CliArgumentParser& operator=(CliArgumentParser&& other) = delete;

  ~CliArgumentParser() = default;

 private:
  CliArgumentParser();
  void Init();

  void ParseArgument(const char* argv);

  std::string_view GetHelp() const;

  ArgumentTrie trie;

  std::array<ArgumentTrie::ValidationExecutionPair, 1> function_pairs;
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_CLI_ARGUMENT_PRASER_H_
