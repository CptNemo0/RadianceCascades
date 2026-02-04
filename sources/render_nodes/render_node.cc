#include "render_node.h"

#include "glad/include/glad/glad.h"

#include <stdexcept>
#include <vector>

namespace rc {

void RenderNode::BindInputs() const {
  for (auto i{0uz}; i < inputs_.size(); ++i) {
    if (!inputs_[i]) {
      throw std::runtime_error(
        "BindPreviousOutput(): An input render target was a nullptr\n");
    }

    inputs_[i]->BindOutput(GL_TEXTURE0 + i);
  }
}

} // namespace rc
