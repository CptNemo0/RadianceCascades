#include "cached_cascades_node.h"

#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"

#include <algorithm>
#include <format>
#include <initializer_list>
#include <memory>
#include <string_view>

#include "constants.h"
#include "image_write.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

CachedCascadesNode::CachedCascadesNode(
  std::string_view name, Parameters& parameters,
  std::initializer_list<RenderNode*> inputs)
  : RenderNode(name, inputs), parameters_(parameters),
    cascade_count_(parameters.cascade_count) {

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

  render_targets_.resize(cascade_count_);
  std::ranges::generate(render_targets_, []() {
    return std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight,
                                          GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
  });
}

void CachedCascadesNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  const Shader* shader = ShaderManager::Instance().Use(
    parameters_.use_sdf ? ShaderManager::ShaderType::kRcSdf
                        : ShaderManager::ShaderType::kRc);
  UpdateUniforms();
  BindInputs();

  for (int i{static_cast<int>(cascade_count_) - 1}; i > -1; --i) {
    if (internal_frame_counter % parameters_.render_frequencies_[i]) {
      continue;
    }

    shader->SetInt("cascade_index", i);
    shader->SetBool("base_level", i == 0);

    render_targets_[i]->Bind();
    render_targets_[i]->Clear();
    if (i != static_cast<int>(cascade_count_) - 1) {
      render_targets_[i + 1]->BindTexture(GL_TEXTURE2);
    }
    rc::Surface::Instance().Draw();
  }

  if (App::Instance().IsMeasuring()) {
    render_targets_[0]->Bind();
    SaveFramebufferToPng(
      std::format("cc\\{}.png", App::Instance().MeasuredFrameIndex()),
      gScreenWidth, gScreenHeight);
  }

  ++internal_frame_counter;
}

void CachedCascadesNode::UpdateUniforms() {
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
