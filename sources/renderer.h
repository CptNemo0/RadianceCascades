#ifndef RC_RENDERER_H_
#define RC_RENDERER_H_

#include <memory>
#include <vector>

#include "aliasing.h"
#include "canvas.h"
#include "render_nodes/render_node.h"

namespace rc {

class Ui;

class Renderer {
  public:
    enum class Mode {
      kGi = 0,
      kRc = 1,
      kModeNumber
    };

    Renderer();

    void Initialize();

    void Render();

    Canvas* canvas() {
      return canvas_.get();
    }

  private:
    friend class Ui;

    Mode mode_ = Mode::kGi;
    std::unique_ptr<rc::Canvas> canvas_;
    std::vector<std::unique_ptr<RenderNode>> nodes_;
    std::vector<RenderNode*> cascades_pipeline_;
    std::vector<RenderNode*> gi_pipeline_;
    i32 stage_to_render_;
};

} // namespace rc

#endif // !RC_RENDERER_H_
