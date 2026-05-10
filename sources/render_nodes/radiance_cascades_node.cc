#include "render_nodes/radiance_cascades_node.h"

#include "app.h"
#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"

#include <format>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <utility>

#include "constants.h"
#include "image_write.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

RadianceCascadesNode::RadianceCascadesNode(
  std::string_view name, RadianceCascadesNode::Parameters& params,
  std::initializer_list<RenderNode*> inputs)
  : RenderNode(name, inputs), parameters_(params),
    render_target_1_(std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                    rc::gScreenHeight, GL_RGBA8,
                                                    GL_RGBA, GL_UNSIGNED_BYTE)),

    render_target_2_(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight,
                                     GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE)) {
  const rc::Shader* shader_rc =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
  shader_rc->SetVec2("resolution",
                     glm::vec2(static_cast<float>(rc::gScreenWidth),
                               static_cast<float>(rc::gScreenHeight)));
  shader_rc->SetInt("color_texture", 0);
  shader_rc->SetInt("sdf_texture", 1);
  shader_rc->SetInt("upper_cascade_texture", 2);

  const rc::Shader* shader_rc_sdf =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kRcSdf);
  shader_rc_sdf->SetVec2("resolution",
                         glm::vec2(static_cast<float>(rc::gScreenWidth),
                                   static_cast<float>(rc::gScreenHeight)));
  shader_rc_sdf->SetInt("color_texture", 0);
  shader_rc_sdf->SetInt("sdf_texture", 1);
  shader_rc_sdf->SetInt("upper_cascade_texture", 2);

  render_targets_[0] = render_target_1_.get();
  render_targets_[1] = render_target_2_.get();
}

void RadianceCascadesNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  const Shader* shader = ShaderManager::Instance().Use(
    parameters_.use_sdf ? ShaderManager::ShaderType::kRcSdf
                        : ShaderManager::ShaderType::kRc);
  UpdateUniforms();
  BindInputs();

  // Last frame is second in the rc_render_targets array.
  for (int i{parameters_.cascade_count - 1}; i > -1; --i) {
    shader->SetInt("cascade_index", i);
    shader->SetBool("base_level", i == 0);
    render_targets_[0]->Bind();
    render_targets_[0]->ClearDefault();
    render_targets_[1]->BindTexture(GL_TEXTURE2);
    rc::Surface::Instance().Draw();
    std::swap(render_targets_[0], render_targets_[1]);
  }

  if (App::Instance().IsMeasuring()) {
    render_targets_[1]->Bind();
    SaveFramebufferToPng(
      std::format("rc\\{}.png", App::Instance().MeasuredFrameIndex()),
      gScreenWidth, gScreenHeight);
  }
}

void RadianceCascadesNode::UpdateUniforms() {
  if (!parameters_.dirty) {
    return;
  }
  const rc::Shader* shader_rc = ShaderManager::Instance().Use(
    parameters_.use_sdf ? ShaderManager::ShaderType::kRcSdf
                        : ShaderManager::ShaderType::kRc);
  shader_rc->SetFloat("base_ray_count", parameters_.base_ray_count);
  shader_rc->SetFloat("cascade_count", parameters_.cascade_count);
  shader_rc->SetFloat("overlap", parameters_.overlap);
  shader_rc->SetInt("step_count", parameters_.step_count);
  shader_rc->SetFloat("proximity_epsilon", parameters_.proximity_epsilon);
  parameters_.dirty = false;
}

} // namespace rc
