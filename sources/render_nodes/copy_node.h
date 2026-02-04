#ifndef RC_RENDER_NODES_COPY_NODE_H_
#define RC_RENDER_NODES_COPY_NODE_H_

#include <initializer_list>
#include <memory>

#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader_manager.h"

namespace rc {

class CopyNode : public RenderNode {
  public:
    explicit CopyNode(ShaderManager::ShaderType copy_shader,
                      std::initializer_list<RenderNode*> inputs)
      : RenderNode(inputs), copy_shader_(copy_shader),
        output_(
          std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)) {
    }

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      output_->BindTexture(texture_slot);
    }

  private:
    ShaderManager::ShaderType copy_shader_ =
      ShaderManager::ShaderType::kSurface;
    std::unique_ptr<RenderTarget> output_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_COPY_NODE_H_
