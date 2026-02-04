#include "render_nodes/global_illumination_node.h"

#include "glad/include/glad/glad.h"

#include <initializer_list>
#include <memory>

#include "app.h"
#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "utility.h"

namespace rc {

GlobalIlluminationNode::GlobalIlluminationNode(
  std::initializer_list<RenderNode*> inputs)
  : RenderNode(inputs), gi_render_target_(std::make_unique<RenderTarget>(
                          rc::gScreenWidth, rc::gScreenHeight)),
    previous_frame_(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)),
    noise_texture_(rc::GetNoiseTexture(rc::gScreenWidth, rc::gScreenHeight)) {
  const Shader* shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
  shader->setInt("color_texture", 0);
  shader->setInt("sdf_texture", 1);
  shader->setInt("previous_frame", 2);
  shader->setInt("noise_texture", 3);
}

void GlobalIlluminationNode::Forward() {
  float time = App::Instance().GetTime() * 1000.f - time_normalizer_;
  if (time > 1000.0f) {
    time_normalizer_ += 1000.0f;
  }

  const Shader* shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
  BindInputs();
  UpdateUniforms();
  shader->setFloat("time", time);
  gi_render_target_->Bind();
  gi_render_target_->Clear();
  previous_frame_->BindTexture(GL_TEXTURE0 + inputs_.size() + 0);
  noise_texture_->BindTexture(GL_TEXTURE0 + inputs_.size() + 1);
  rc::Surface::Instnace().Draw();

  rc::ShaderManager::Instance().Use(ShaderManager::ShaderType::kSurface);
  previous_frame_->Bind();
  previous_frame_->Clear();
  gi_render_target_->BindTexture(GL_TEXTURE0);
  rc::Surface::Instnace().Draw();
}

void GlobalIlluminationNode::UpdateUniforms() {
  if (!parameters_.dirty) {
    return;
  }
  const Shader* shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
  shader->setInt("step_count", parameters_.step_count);
  shader->setFloat("proximity_epsilon", parameters_.proximity_epsilon);
  shader->setInt("ray_count", parameters_.ray_count);
  shader->setFloat("one_over_ray_count", parameters_.one_over_ray_count);
  shader->setFloat("angle_step", parameters_.angle_step);
  parameters_.dirty = false;
}

} // namespace rc
