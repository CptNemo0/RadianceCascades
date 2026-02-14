#ifndef RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_
#define RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_

#include <initializer_list>
#include <memory>
#include <numbers>
#include <string_view>

#include "aliasing.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "texture.h"

namespace rc {

// GlobalIlluminationNode provides a naive global illumination approach
// solution. It takes in Canvas and Sdf nodes as input.
// Before every render dirty flag of parameters_ is checked. If it's set the
// uniforms are updated with data present in the structure.
class GlobalIlluminationNode : public RenderNode {
  public:
    struct Parameters {
        bool dirty{true};
        float noise_amount = 0.5;
        i32 step_count = 128;
        f32 proximity_epsilon = 0.00001f;
        i32 ray_count = 32;
        f32 one_over_ray_count = 1.0f / ray_count;
        f32 angle_step =
          static_cast<float>(std::numbers::pi) * 2.0f * one_over_ray_count;
    };

    GlobalIlluminationNode(std::string_view name, Parameters& params,
                           std::initializer_list<RenderNode*> inputs);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      previous_frame_->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms();
    Parameters& parameters_;
    std::unique_ptr<RenderTarget> gi_render_target_;
    std::unique_ptr<RenderTarget> previous_frame_;
    std::unique_ptr<Texture> noise_texture_;
    float time_normalizer_ = 0.0f;
};

} // namespace rc

#endif // !RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_
