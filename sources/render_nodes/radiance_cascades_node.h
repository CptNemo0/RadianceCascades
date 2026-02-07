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
        i32 step_count = 32;
        i32 base_ray_count = 4;
        i32 cascade_count = 5;
        f32 overlap = 0.17f;
        f32 magic = 0.01f;
        f32 proximity_epsilon = 0.0001f;
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
