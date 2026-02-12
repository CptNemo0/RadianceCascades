#ifndef RC_RENDER_NODES_FIRE_NODE_H_
#define RC_RENDER_NODES_FIRE_NODE_H_

#include "glm/fwd.hpp"

#include <memory>

#include "flame_generator.h"
#include "render_nodes/render_node.h"
#include "render_target.h"

namespace rc {

class FireNode : public RenderNode {
  public:
    FireNode(FlameGenerator& flame_generator, RenderNode* input);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      output_texture_->BindTexture(texture_slot);
    }

  private:
    FlameGenerator& flame_generator_;
    std::unique_ptr<RenderTarget> output_texture_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_FIRE_NODE_H_
