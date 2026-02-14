#ifndef RC_RENDER_NODES_H_
#define RC_RENDER_NODES_H_

#include <string_view>

#include "canvas.h"
#include "render_nodes/render_node.h"
#include "timed_scope.h"

namespace rc {

// CanvasNode is a wrapper around a Canvas class that allows for painting.
// It's supposed to be a start of a pipeline so it doesn't accept inputs.
class CanvasNode : public RenderNode {
  public:
    explicit CanvasNode(std::string_view name, Canvas& canvas)
      : RenderNode(name, {}), canvas_(canvas) {
    }

    virtual void Forward() override {
      TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
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
