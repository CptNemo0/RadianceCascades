#include "copy_node.h"

#include "render_target.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

void CopyNode::Forward() {
  ShaderManager::Instance().Use(copy_shader_);
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
