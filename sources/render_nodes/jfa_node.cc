#include "render_nodes/jfa_node.h"

#include "glad/include/glad/glad.h"

#include <cmath>
#include <memory>
#include <utility>

#include "constants.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

JfaNode::JfaNode(RenderNode* initializer_node)
  : RenderNode({initializer_node}),
    render_target_1(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)),
    render_target_2(
      std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)) {

  const Shader* jfa_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kJfa);
  jfa_shader->setFloat("width", rc::gScreenWidth);
  jfa_shader->setFloat("height", rc::gScreenHeight);
  jfa_shader->setFloat("one_over_width", rc::gOneOverWidth);
  jfa_shader->setFloat("one_over_height", rc::gOneOverHeight);
  render_targets_[0] = render_target_1.get();
  render_targets_[1] = render_target_2.get();
}

void JfaNode::Forward() {
  Initialize();
  const Shader* jfa_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kJfa);
  jfa_swaps = 0;
  for (auto i{0uz}; i < rc::gJfaSteps; ++i) {
    // -1 to start from N / 2
    jfa_shader->setInt("step_size", std::pow(2, rc::gJfaSteps - i - 1));

    render_targets_[1]->Bind();
    render_targets_[1]->Clear();
    render_targets_[0]->BindTexture(GL_TEXTURE0);
    rc::Surface::Instnace().Draw();
    std::swap(render_targets_[0], render_targets_[1]);
    jfa_swaps++;
  }
}

void JfaNode::BindOutput(int texture_slot) const {
  render_targets_[1]->BindTexture(texture_slot);
}

void JfaNode::Initialize() {
  const auto* _ =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kSurface);
  inputs_[0]->BindOutput(GL_TEXTURE0);
  render_target_1->Bind();
  render_target_1->Clear();
  Surface::Instnace().Draw();
  render_targets_[0] = render_target_1.get();
  render_targets_[1] = render_target_2.get();
  jfa_swaps = 0;
}

} // namespace rc
