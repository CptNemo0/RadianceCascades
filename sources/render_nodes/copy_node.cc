#include "copy_node.h"

#include <initializer_list>
#include <memory>
#include <string_view>

#include "aliasing.h"
#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

CopyNode::CopyNode(std::string_view name,
                   std::initializer_list<RenderNode*> inputs, bool to_screen)
  : RenderNode(name, inputs),
    output_(to_screen ? nullptr
                      : std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                       rc::gScreenHeight)) {
}

CopyNode::CopyNode(std::string_view name, ShaderType copy_shader,
                   std::initializer_list<RenderNode*> inputs, bool to_screen)
  : RenderNode(name, inputs), copy_shader_(copy_shader),
    output_(to_screen ? nullptr
                      : std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                       rc::gScreenHeight)) {
}

CopyNode::CopyNode(std::string_view name, ShaderType copy_shader,
                   std::initializer_list<RenderNode*> inputs, i32 bits,
                   i32 format, i32 type)
  : RenderNode(name, inputs), copy_shader_(copy_shader),
    output_(std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight,
                                           bits, format, type)) {
}

void CopyNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
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
