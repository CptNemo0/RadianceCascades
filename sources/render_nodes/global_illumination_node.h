#ifndef RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_
#define RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_

#include <initializer_list>
#include <memory>
#include <numbers>

#include "aliasing.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "texture.h"

namespace rc {

class GlobalIlluminationNode : public RenderNode {
  public:
    struct Parameters {
        bool dirty{true};
        i32 step_count = 128;
        f32 proximity_epsilon = 0.00001f;
        i32 ray_count = 32;
        f32 one_over_ray_count = 1.0f / static_cast<float>(ray_count);
        f32 angle_step =
          static_cast<float>(std::numbers::pi) * one_over_ray_count;
    };

    GlobalIlluminationNode(std::initializer_list<RenderNode*> inputs);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      previous_frame_->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms();

    Parameters parameters_;
    std::unique_ptr<RenderTarget> gi_render_target_;
    std::unique_ptr<RenderTarget> previous_frame_;
    std::unique_ptr<Texture> noise_texture_;
    float time_normalizer_ = 0.0;
};

} // namespace rc

#endif // !RC_RENDER_NODE_GLOBAL_ILLUMINATION_NODE_H_
