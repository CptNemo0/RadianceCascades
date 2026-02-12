#include "render_nodes/fire_node.h"
#include "constants.h"
#include "flame_generator.h"
#include "glad/include/glad/glad.h"
#include "glm/ext/vector_float2.hpp"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include <memory>

namespace rc {

FireNode::FireNode(FlameGenerator& flame_generator, RenderNode* input)
  : RenderNode({input}), flame_generator_(flame_generator),
    output_texture_(
      std::make_unique<RenderTarget>(gScreenHeight, gScreenHeight)) {
}

void FireNode::Forward() {
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
