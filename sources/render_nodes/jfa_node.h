#ifndef RC_RENDER_NODES_JFA_NODE_H_
#define RC_RENDER_NODES_JFA_NODE_H_

#include "render_nodes/render_node.h"

#include <array>
#include <memory>

#include "aliasing.h"
#include "render_target.h"

namespace rc {

// Jump flood algorithm shader. Ping pong shader that fills space with the
// nearest uv data. Used to calculate signed distance field. Input should be a
// picture where each pixel with non-zero alpha has components r and g
// substituted with uv value.
class JfaNode : public RenderNode {
  public:
    explicit JfaNode(RenderNode* initializer_node);

    virtual void Forward() override;

    virtual void BindOutput(int texture_slot) const override;

  private:
    void Initialize();

    rc::u32 jfa_swaps{0};
    std::unique_ptr<rc::RenderTarget> render_target_1;
    std::unique_ptr<rc::RenderTarget> render_target_2;
    std::array<rc::RenderTarget*, 2> render_targets_;
};

} // namespace rc

#endif // !RC_RENDER_NODES_JFA_NODE_H_
