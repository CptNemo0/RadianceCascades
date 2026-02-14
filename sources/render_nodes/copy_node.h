#ifndef RC_RENDER_NODES_COPY_NODE_H_
#define RC_RENDER_NODES_COPY_NODE_H_

#include <initializer_list>
#include <memory>
#include <string_view>

#include "render_nodes/render_node.h"
#include "render_target.h"

namespace rc {

// This node performs a copy of the input texture to it's internal render
// target. However a different shader can be provided, that will in turn also
// transform the pixels. For example kSdf, kUvColorspace
// This node can also be used to draw to screen. To do that pass true to last
// argument of a constructor.
class CopyNode : public RenderNode {
  public:
    CopyNode(std::string_view name, std::initializer_list<RenderNode*> inputs,
             bool to_screen = false);

    CopyNode(std::string_view name, ShaderType copy_shader,
             std::initializer_list<RenderNode*> inputs, bool to_screen = false);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      output_->BindTexture(texture_slot);
    }

  private:
    // Stored shader variant. If no is passed to constructor a normal copy is
    // performed.
    ShaderType copy_shader_ = ShaderType::kSurface;

    // If nullptr, the shader will draw to screen instead.
    std::unique_ptr<RenderTarget> output_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_COPY_NODE_H_
