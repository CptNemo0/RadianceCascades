#ifndef RC_RENDERER_H_
#define RC_RENDERER_H_

#include <memory>
#include <vector>

#include "aliasing.h"
#include "canvas.h"
#include "flame_generator.h"
#include "render_nodes/cached_cascades_node.h"
#include "render_nodes/global_illumination_node.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"

namespace rc {

class Ui;

class Renderer {
  public:
    enum class Mode {
      kGi = 0,
      kRc = 1,
      kCachedRc = 2,
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

    Mode mode_ = Mode::kRc;
    i32 stage_to_render_;

    std::unique_ptr<Canvas> canvas_;
    std::unique_ptr<FlameGenerator> flame_generator_;

    std::vector<std::unique_ptr<RenderNode>> nodes_;
    std::vector<RenderNode*> cascades_pipeline_;
    std::vector<RenderNode*> gi_pipeline_;
    std::vector<RenderNode*> cached_rc_pipeline_;

    GlobalIlluminationNode::Parameters global_illumination_params_;
    RadianceCascadesNode::Parameters cascades_params_;
    CachedCascadesNode::Parameters cached_params_;
};

} // namespace rc

#endif // !RC_RENDERER_H_
