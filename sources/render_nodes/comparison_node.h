#ifndef RC_RENDER_NODES_COMPARISON_NODE_H_
#define RC_RENDER_NODES_COMPARISON_NODE_H_

#include "constants.h"
#include "glad/include/glad/glad.h"

#include <initializer_list>
#include <memory>
#include <string_view>

#include "aliasing.h"
#include "render_nodes/render_node.h"
#include "render_target.h"

namespace rc {

// This node performs a copy of the input texture to it's internal render
// target. However a different shader can be provided, that will in turn also
// transform the pixels. For example kSdf, kUvColorspace
// This node can also be used to draw to screen. To do that pass true to last
// argument of a constructor.
class ComparisonNode : public RenderNode {
  public:
    ComparisonNode(std::string_view name,
                   std::initializer_list<RenderNode*> inputs);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      output_->BindTexture(texture_slot);
    }

  private:
    // If nullptr, the shader will draw to screen instead.
    std::unique_ptr<RenderTarget> output_{
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)};
};

} // namespace rc

#endif // !RC_RENDER_NODES_COMPARISON_NODE_H_
