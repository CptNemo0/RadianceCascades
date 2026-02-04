#ifndef RC_RENDERER_H_
#define RC_RENDERER_H_

#include <memory>
#include <vector>

#include "canvas.h"
#include "render_nodes/render_node.h"

namespace rc {

class Renderer {
  public:
    enum class Mode {
      kGi,
      kRc
    };

    Renderer();

    void Initialize();

    void Render();

    Canvas& canvas() {
      return canvas_;
    }

  private:
    Mode mode_ = Mode::kRc;
    rc::Canvas canvas_;
    std::vector<std::unique_ptr<RenderNode>> nodes_;
    std::vector<RenderNode*> cascades_pipeline_;
    std::vector<RenderNode*> gi_pipeline_;
};

} // namespace rc

#endif // !RC_RENDERER_H_
