#include "comparison_node.h"

#include "glad/include/glad/glad.h"

#include <initializer_list>
#include <string_view>

#include "render_nodes/render_node.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

ComparisonNode::ComparisonNode(std::string_view name,
                               std::initializer_list<RenderNode*> inputs)
    : RenderNode(name, inputs) {};

void ComparisonNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  const auto* shader = ShaderManager::Instance().Use(ShaderType::kCompare);
  shader->SetInt("ground_truth", 0);
  shader->SetInt("prediction", 1);
  BindInputs();
  output_->Bind();
  Surface::Instance().Draw();
};

}  // namespace rc
