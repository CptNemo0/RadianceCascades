#ifndef RC_RENDER_NODES_H_
#define RC_RENDER_NODES_H_

#include "canvas.h"
#include "render_nodes/render_node.h"

namespace rc {

class CanvasNode : public RenderNode {
  public:
    explicit CanvasNode(Canvas& canvas) : RenderNode({}), canvas_(canvas) {
    }

    virtual void Forward() override {
      canvas_.Draw();
    }

    virtual void BindOutput(int texture_slot) const override {
      canvas_.BindTexture(texture_slot);
    }

  private:
    Canvas& canvas_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_H_
