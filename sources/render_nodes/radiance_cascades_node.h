#ifndef RC_RENDER_NODES_RADIANCE_CASCADES_NODE_H_
#define RC_RENDER_NODES_RADIANCE_CASCADES_NODE_H_

#include "constants.h"
#include "render_nodes/render_node.h"

#include "aliasing.h"
#include "render_target.h"
#include <array>
#include <cmath>
#include <initializer_list>
#include <memory>

namespace rc {

// Radiance Cascades based global illumination shader.
// solution. It takes in Canvas and Sdf nodes as input.
// Before every render dirty flag of parameters_ is checked. If it's set the
// uniforms are updated with data present in the structure.
class RadianceCascadesNode : public RenderNode {
  public:
    struct Parameters {
        i32 base_ray_count = 64;
        i32 cascade_count = static_cast<int>(
          std::ceil(std::log(std::sqrt(rc::gScreenWidth * rc::gScreenWidth +
                                       rc::gScreenHeight * rc::gScreenHeight)) /
                    std::log(rc::gBaseRayCount)));

        f32 s_rgb = 2.1;
        bool dirty = true;
    };

    RadianceCascadesNode(Parameters& parameters,
                         std::initializer_list<RenderNode*> inputs);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      render_targets_[1]->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms();
    Parameters& parameters_;
    std::unique_ptr<RenderTarget> render_target_1_;
    std::unique_ptr<RenderTarget> render_target_2_;
    std::array<RenderTarget*, 2> render_targets_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_RADIANCE_CASCADES_NODE_H_
