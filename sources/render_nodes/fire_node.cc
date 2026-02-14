#include "render_nodes/fire_node.h"

#include "glad/include/glad/glad.h"
#include "glm/ext/vector_float2.hpp"

#include <memory>
#include <string_view>

#include "constants.h"
#include "flame_generator.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

FireNode::FireNode(std::string_view name, FlameGenerator& flame_generator,
                   RenderNode* input)
  : RenderNode(name, {input}), flame_generator_(flame_generator),
    output_texture_(
      std::make_unique<RenderTarget>(gScreenHeight, gScreenHeight)) {
}

void FireNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  flame_generator_.RenderFlames();
  const Shader* add_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kOverlay);
  add_shader->setInt("texture_1", 0);
  add_shader->setInt("texture_2", 1);
  add_shader->setVec2("offset_1", glm::vec2(0.0f, 0.0f));
  add_shader->setVec2("offset_2", glm::vec2(0.0f, 0.0f));
  output_texture_->Bind();
  output_texture_->Clear();
  flame_generator_.BindTexture(GL_TEXTURE0);
  inputs_[0]->BindOutput(GL_TEXTURE1);
  Surface::Instnace().Draw();
}

} // namespace rc
