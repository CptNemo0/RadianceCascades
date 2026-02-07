#include "render_nodes/radiance_cascades_node.h"

#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"

#include <initializer_list>
#include <memory>
#include <utility>

#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

RadianceCascadesNode::RadianceCascadesNode(
  RadianceCascadesNode::Parameters& params,
  std::initializer_list<RenderNode*> inputs)
  : RenderNode(inputs), parameters_(params),
    render_target_1_(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)),
    render_target_2_(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)) {
  const rc::Shader* shader_rc =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
  shader_rc->setVec2("resolution",
                     glm::vec2(static_cast<float>(rc::gScreenWidth),
                               static_cast<float>(rc::gScreenHeight)));
  shader_rc->setInt("sceneTexture", 0);
  shader_rc->setInt("distanceTexture", 1);
  shader_rc->setInt("lastTexture", 2);
  render_targets_[0] = render_target_1_.get();
  render_targets_[1] = render_target_2_.get();
}

void RadianceCascadesNode::Forward() {
  const Shader* shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
  UpdateUniforms();
  BindInputs();

  // Last frame is second in the rc_render_targets array.
  for (int i{parameters_.cascade_count - 1}; i > -1; --i) {
    shader->setInt("cascade_index", i);
    shader->setBool("base_level", i == 0);
    render_targets_[0]->Bind();
    render_targets_[0]->ClearDefault();
    render_targets_[1]->BindTexture(GL_TEXTURE2);
    rc::Surface::Instnace().Draw();
    std::swap(render_targets_[0], render_targets_[1]);
  }
}

void RadianceCascadesNode::UpdateUniforms() {
  if (!parameters_.dirty) {
    return;
  }
  const rc::Shader* shader_rc =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
  shader_rc->setFloat("base_ray_count", parameters_.base_ray_count);
  shader_rc->setFloat("cascadeCount", parameters_.cascade_count);
  shader_rc->setFloat("overlap", parameters_.overlap);
  shader_rc->setFloat("magic", parameters_.magic);
  shader_rc->setInt("step_count", parameters_.step_count);
  shader_rc->setInt("proximity_epsilon", parameters_.proximity_epsilon);
  parameters_.dirty = false;
}

} // namespace rc
