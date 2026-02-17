#ifndef RC_RENDER_NODES_CACHED_CASCADES_NODE_H_
#define RC_RENDER_NODES_CACHED_CASCADES_NODE_H_

#include <array>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <vector>

#include "aliasing.h"
#include "constants.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"
#include "render_target.h"

namespace rc {

class CachedCascadesNode : public RenderNode {
  public:
    struct Parameters : public RadianceCascadesNode::Parameters {
        std::vector<i32> render_frequencies_;
    };

    CachedCascadesNode(std::string_view name, Parameters& parameters,
                       std::initializer_list<RenderNode*> inputs);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override {
      render_targets_[0]->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms();
    Parameters& parameters_;
    std::vector<std::unique_ptr<RenderTarget>> render_targets_;

    const u32 cascade_count_;
    u32 internal_frame_counter{};
};

} // namespace rc

#endif // !RC_RENDER_NODES_CACHED_CASCADES_NODE_H_
