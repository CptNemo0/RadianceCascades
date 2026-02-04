#ifndef RC_RENDER_NODE_H_
#define RC_RENDER_NODE_H_

#include <initializer_list>
#include <vector>

namespace rc {

class RenderNode {
  public:
    RenderNode() = default;
    virtual ~RenderNode() = default;

    // Performs this node's rendering routine.
    virtual void Forward() = 0;

    // Binds output texture to the provided texture slot.
    virtual void BindOutput(int texture_slot) const = 0;

    void BindInputs() const;

  protected:
    RenderNode(std::initializer_list<RenderNode*> inputs) : inputs_(inputs) {};
    std::vector<RenderNode*> inputs_{};
};

} // namespace rc

#endif // !RC_RENDER_NODE_H_
