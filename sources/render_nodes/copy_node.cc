#include "copy_node.h"

#include <initializer_list>
#include <memory>

#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

CopyNode::CopyNode(std::initializer_list<RenderNode*> inputs, bool to_screen)
  : RenderNode(inputs),
    output_(to_screen ? nullptr
                      : std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                       rc::gScreenHeight)) {
}

CopyNode::CopyNode(ShaderType copy_shader,
                   std::initializer_list<RenderNode*> inputs, bool to_screen)
  : RenderNode(inputs), copy_shader_(copy_shader),
    output_(to_screen ? nullptr
                      : std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                       rc::gScreenHeight)) {
}

void CopyNode::Forward() {
  const auto* _ = ShaderManager::Instance().Use(copy_shader_);
  if (output_) {
    output_->Bind();
    output_->Clear();
  } else {
    RenderTarget::BindDefault();
    RenderTarget::ClearDefault();
  }
  BindInputs();
  rc::Surface::Instnace().Draw();
}

} // namespace rc
