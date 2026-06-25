#ifndef RADIANCE_CASCADES_TRIE_H_
#define RADIANCE_CASCADES_TRIE_H_

#include <cctype>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace rc {

class ArgumentTrie {
 public:
  struct ValidationExecutionPair {
    void (*Execute)(std::string_view);
    bool (*Validate)(std::string_view);
  };

  struct Node {
    std::unordered_map<char, std::unique_ptr<Node>> next;
    Node* previous{nullptr};
    ValidationExecutionPair* functions{nullptr};
    bool end_of_word{false};
    bool requires_value{false};

    explicit Node(Node* parent) { previous = parent; }

    char ValidateNormalize(char character) const {
      // If character is not representable by u8 std::lowercase will UB.
      if (character < 0) {
        throw std::runtime_error{
            std::format("Character: {} is not a letter.", character)};
      }

      const char lowercase{static_cast<char>(std::tolower(character))};

      if (lowercase < 'a' || lowercase > 'z') {
        throw std::runtime_error{
            std::format("Character: {} is not a letter.", character)};
      }
      return lowercase;
    }

    Node* AdvanceTo(char character) const {
      const char lowercase = ValidateNormalize(character);
      return next.contains(lowercase) ? next.at(lowercase).get() : nullptr;
    };

    Node* InsertNewCharacter(char character) {
      const char lowercase = ValidateNormalize(character);
      next[lowercase] = std::make_unique<Node>(this);
      return next[lowercase].get();
    }

    void MarkEndOfWord() { end_of_word = true; };

    void RequiresValues() { requires_value = true; };

    void SetFunctionPair(ValidationExecutionPair* ve_pair) {
      functions = ve_pair;
    }
  };

  ArgumentTrie() : start_node_(std::make_unique<Node>(nullptr)) {}

  void InsertArgument(std::string_view argument_name,
                      bool requires_value,
                      ValidationExecutionPair* ve_pair) {
    Node* current = start_node_.get();
    for (const char& character : argument_name) {
      if (Node* next = current->AdvanceTo(character); next) {
        current = next;
      } else {
        current = current->InsertNewCharacter(character);
      }
    }
    current->MarkEndOfWord();
    if (requires_value) {
      current->RequiresValues();
    }
    if (ve_pair) {
      current->SetFunctionPair(ve_pair);
    }
  };

  const Node* GetIterator() { return start_node_.get(); };

 private:
  std::unique_ptr<Node> start_node_;
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_TRIE_H_
