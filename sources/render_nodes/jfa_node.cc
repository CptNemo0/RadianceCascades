#include "render_nodes/jfa_node.h"

#include "glad/include/glad/glad.h"

#include <cmath>
#include <memory>
#include <string_view>
#include <utility>

#include "aliasing.h"
#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

JfaNode::JfaNode(std::string_view name, RenderNode* initializer_node)
    : RenderNode(name, {initializer_node}),
      render_target_1(std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                     rc::gScreenHeight,
                                                     GL_RG32F,
                                                     GL_RG,
                                                     GL_FLOAT)),
      render_target_2(std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                     rc::gScreenHeight,
                                                     GL_RG32F,
                                                     GL_RG,
                                                     GL_FLOAT)) {
  const Shader* jfa_shader = ShaderManager::Instance().Use(ShaderType::kJfa);
  jfa_shader->SetFloat("width", rc::gScreenWidth);
  jfa_shader->SetFloat("height", rc::gScreenHeight);
  jfa_shader->SetFloat("one_over_width", rc::gOneOverWidth);
  jfa_shader->SetFloat("one_over_height", rc::gOneOverHeight);
  render_targets_[0] = render_target_1.get();
  render_targets_[1] = render_target_2.get();
}

void JfaNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  Initialize();
  const Shader* jfa_shader = ShaderManager::Instance().Use(ShaderType::kJfa);
  u32 steps = std::pow(2, rc::gJfaSteps);
  for (auto i{0uz}; i < rc::gJfaSteps; ++i) {
    steps /= 2;
    jfa_shader->SetInt("step_size", steps);

    render_targets_[1]->Bind();
    render_targets_[1]->Clear();
    render_targets_[0]->BindTexture(GL_TEXTURE0);
    rc::Surface::Instance().Draw();
    std::swap(render_targets_[0], render_targets_[1]);
  }
}

void JfaNode::BindOutput(int texture_slot) const {
  render_targets_[0]->BindTexture(texture_slot);
}

void JfaNode::Initialize() {
  const auto* _ = ShaderManager::Instance().Use(ShaderType::kSurface);
  inputs_[0]->BindOutput(GL_TEXTURE0);
  render_target_1->Bind();
  render_target_1->Clear();
  Surface::Instance().Draw();
  render_targets_[0] = render_target_1.get();
  render_targets_[1] = render_target_2.get();
}

}  // namespace rc
