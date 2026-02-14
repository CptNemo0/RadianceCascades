#include "renderer.h"

#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

#include "aliasing.h"
#include "canvas.h"
#include "constants.h"
#include "flame_generator.h"
#include "glad/include/glad/glad.h"
#include "imgui.h"
#include "render_nodes/canvas_node.h"
#include "render_nodes/copy_node.h"
#include "render_nodes/fire_node.h"
#include "render_nodes/global_illumination_node.h"
#include "render_nodes/jfa_node.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

Renderer::Renderer()
  : canvas_(std::make_unique<Canvas>(rc::gScreenHeight, rc::gScreenHeight, 10)),
    flame_generator_(std::make_unique<FlameGenerator>()) {
}

void Renderer::Initialize() {
  std::unique_ptr<rc::CanvasNode> canvas_node =
    std::make_unique<rc::CanvasNode>("CanvasNode", *(canvas_.get()));

  std::unique_ptr<rc::FireNode> flame_node = std::make_unique<rc::FireNode>(
    "FireNode", *(flame_generator_.get()), canvas_node.get());

  std::unique_ptr<rc::CopyNode> uv_colorspace_node =
    std::make_unique<rc::CopyNode>(
      "UVColorspaceNode", rc::ShaderManager::ShaderType::kUvColorspace,
      std::initializer_list<rc::RenderNode*>{flame_node.get()});

  std::unique_ptr<rc::JfaNode> jfa_node =
    std::make_unique<rc::JfaNode>("JFANode", uv_colorspace_node.get());

  std::unique_ptr<rc::CopyNode> sdf_node = std::make_unique<rc::CopyNode>(
    "SDFNode", rc::ShaderManager::ShaderType::kSdf,
    std::initializer_list<rc::RenderNode*>{jfa_node.get()});

  std::unique_ptr<rc::GlobalIlluminationNode> gi_node =
    std::make_unique<rc::GlobalIlluminationNode>(
      "GlobalIlluminationNode", global_illumination_params_,
      std::initializer_list<rc::RenderNode*>{flame_node.get(), sdf_node.get()});

  std::unique_ptr<rc::RadianceCascadesNode> rc_node =
    std::make_unique<rc::RadianceCascadesNode>(
      "RadianceCascadesNode", cascades_params_,
      std::initializer_list<rc::RenderNode*>{flame_node.get(), sdf_node.get()});

  cascades_pipeline_.push_back(canvas_node.get());
  cascades_pipeline_.push_back(flame_node.get());
  cascades_pipeline_.push_back(uv_colorspace_node.get());
  cascades_pipeline_.push_back(jfa_node.get());
  cascades_pipeline_.push_back(sdf_node.get());
  cascades_pipeline_.push_back(rc_node.get());

  gi_pipeline_.push_back(canvas_node.get());
  gi_pipeline_.push_back(flame_node.get());
  gi_pipeline_.push_back(uv_colorspace_node.get());
  gi_pipeline_.push_back(jfa_node.get());
  gi_pipeline_.push_back(sdf_node.get());
  gi_pipeline_.push_back(gi_node.get());

  nodes_.push_back(std::move(canvas_node));
  nodes_.push_back(std::move(flame_node));
  nodes_.push_back(std::move(uv_colorspace_node));
  nodes_.push_back(std::move(jfa_node));
  nodes_.push_back(std::move(sdf_node));
  nodes_.push_back(std::move(gi_node));
  nodes_.push_back(std::move(rc_node));

  stage_to_render_ = gi_pipeline_.size() - 1;
}

void Renderer::Render() {
  std::vector<RenderNode*>& pipeline = [this]() -> std::vector<RenderNode*>& {
    switch (mode_) {
    case Mode::kGi:
      return gi_pipeline_;
    case Mode::kRc:
      return cascades_pipeline_;
    case Mode::kModeNumber:
      return cascades_pipeline_;
    }
  }();

  for (const auto& node : pipeline) {
    node->Forward();
  }

  ShaderManager::Instance().Use(ShaderManager::ShaderType::kSurface);
  RenderTarget::BindDefault();
  RenderTarget::ClearDefault();
  [pipeline, this]() {
    if (stage_to_render_ < static_cast<i32>(gi_pipeline_.size())) {
      return pipeline[stage_to_render_];
    } else {
      return pipeline.back();
    }
  }()
    ->BindOutput(GL_TEXTURE0);
  Surface::Instnace().Draw();
}

} // namespace rc
