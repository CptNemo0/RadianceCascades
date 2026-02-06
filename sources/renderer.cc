#include "renderer.h"

#include "canvas.h"
#include "constants.h"
#include "render_nodes/canvas_node.h"
#include "render_nodes/copy_node.h"
#include "render_nodes/global_illumination_node.h"
#include "render_nodes/jfa_node.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"
#include "shader_manager.h"
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

namespace rc {

Renderer::Renderer()
  : canvas_(
      std::make_unique<Canvas>(rc::gScreenHeight, rc::gScreenHeight, 10)) {
}

void Renderer::Initialize() {
  std::unique_ptr<rc::CanvasNode> canvas_node =
    std::make_unique<rc::CanvasNode>(*(canvas_.get()));

  std::unique_ptr<rc::CopyNode> uv_colorspace_node =
    std::make_unique<rc::CopyNode>(
      rc::ShaderManager::ShaderType::kUvColorspace,
      std::initializer_list<rc::RenderNode*>{canvas_node.get()});

  std::unique_ptr<rc::JfaNode> jfa_node =
    std::make_unique<rc::JfaNode>(uv_colorspace_node.get());

  std::unique_ptr<rc::CopyNode> sdf_node = std::make_unique<rc::CopyNode>(
    rc::ShaderManager::ShaderType::kSdf,
    std::initializer_list<rc::RenderNode*>{jfa_node.get()});

  std::unique_ptr<rc::GlobalIlluminationNode> gi_node =
    std::make_unique<rc::GlobalIlluminationNode>(
      std::initializer_list<rc::RenderNode*>{canvas_node.get(),
                                             sdf_node.get()});

  std::unique_ptr<rc::RadianceCascadesNode> rc_node =
    std::make_unique<rc::RadianceCascadesNode>(
      std::initializer_list<rc::RenderNode*>{canvas_node.get(),
                                             sdf_node.get()});

  cascades_pipeline_.push_back(canvas_node.get());
  cascades_pipeline_.push_back(uv_colorspace_node.get());
  cascades_pipeline_.push_back(jfa_node.get());
  cascades_pipeline_.push_back(sdf_node.get());
  cascades_pipeline_.push_back(rc_node.get());

  gi_pipeline_.push_back(canvas_node.get());
  gi_pipeline_.push_back(uv_colorspace_node.get());
  gi_pipeline_.push_back(jfa_node.get());
  gi_pipeline_.push_back(sdf_node.get());
  gi_pipeline_.push_back(gi_node.get());

  nodes_.push_back(std::move(canvas_node));
  nodes_.push_back(std::move(uv_colorspace_node));
  nodes_.push_back(std::move(jfa_node));
  nodes_.push_back(std::move(sdf_node));
  nodes_.push_back(std::move(gi_node));
  nodes_.push_back(std::move(rc_node));
}

void Renderer::Render() {
  std::vector<RenderNode*>& pipeline = [this]() -> std::vector<RenderNode*>& {
    switch (mode_) {
    case Mode::kGi:
      return gi_pipeline_;
    case Mode::kRc:
      return cascades_pipeline_;
    }
  }();

  CopyNode copy_node{{pipeline.back()}, true};

  for (const auto& node : pipeline) {
    node->Forward();
  }
  copy_node.Forward();
}

} // namespace rc
