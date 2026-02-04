#ifndef RC_RENDER_NODE_H_
#define RC_RENDER_NODE_H_

#include <initializer_list>
#include <vector>

#include "shader_manager.h"

namespace rc {

// RenderNodes is a base class for all classes in this directory.
// This design is based on rather prevalent idea of shader graph.
// All classes inheriting after RenderNode are supposed to interface easily with
// each other by passing one's output to other's input.
class RenderNode {
  public:
    RenderNode() = default;
    virtual ~RenderNode() = default;

    // Performs rendering logic of the node.
    virtual void Forward() = 0;

    // Binds render target's texture to the provided texture slot. Used by
    // BindInputs.
    virtual void BindOutput(int texture_slot) const = 0;

    // Binds  render target textures of all of the inputs to consecutive texture
    // slots starting from GL_TEXTURE0.
    void BindInputs() const;

  protected:
    using ShaderType = ShaderManager::ShaderType;

    RenderNode(std::initializer_list<RenderNode*> inputs) : inputs_(inputs) {};

    // RenderTargets of those RenderNodes are going to be treated as input
    // textures for the node.
    std::vector<RenderNode*> inputs_{};
};

} // namespace rc

#endif // !RC_RENDER_NODE_H_
