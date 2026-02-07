#include "canvas.h"

#include <algorithm>
#include <array>
#include <chrono>

#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"

#include "aliasing.h"
#include "constants.h"
#include "glm/geometric.hpp"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"

namespace rc {

Canvas::Canvas(u64 height, u64 width, u64 brush_radius, glm::vec3 brush_color)
  : height_(height), width_(width),
    brush_radius_(std::clamp(brush_radius, 0uz, gMaxBrushRadius)),
    brush_color_(brush_color) {
}

void Canvas::RegisterPoint(float x, float y) {
  // Reverse y position. Without it moving mouse up results in the drawing
  // position going down.
  y = height_ - y;

  const glm::vec2 registered_point{x, y};
  const auto now = std::chrono::high_resolution_clock::now();
  previous_position_ = selected_position_;

  if (first_) [[unlikely]] {
    selected_position_ = registered_point;
    first_ = false;
    last_draw_ = now;
    return;
  }

  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_draw_)
        .count() < 63) {

    const glm::vec2 diff = registered_point - previous_position_;
    // In the case where points are too close, select the same point as
    // previously.
    if (glm::dot(diff, diff) < 20.0f) {
      selected_position_ = previous_position_;
      last_draw_ = now;
      return;
    }

    selected_position_ =
      previous_position_ +
      static_cast<float>(brush_radius_) * 0.5f * glm::normalize(diff);
    last_draw_ = now;
    return;
  }

  selected_position_ = registered_point;
  last_draw_ = now;
}

void Canvas::Draw() {
  if (first_) {
    return;
  }
  const rc::Shader* canvas_shader =
    ShaderManager::Instance().GetShader(ShaderManager::ShaderType::kCanvas);
  canvas_shader->use();
  render_targets_[0]->Bind();
  render_targets_[0]->Clear();
  render_targets_[1]->BindTexture(GL_TEXTURE0);
  canvas_shader->setFloat("width", static_cast<float>(width_));
  canvas_shader->setFloat("height_", static_cast<float>(height_));
  canvas_shader->setFloat("brush_radius", brush_radius_);
  canvas_shader->setVec2("position", selected_position_);
  canvas_shader->setVec3("brush_color", brush_color_);
  canvas_shader->setBool("eraser", eraser_);
  Surface::Instnace().Draw();

  std::swap(render_targets_[0], render_targets_[1]);

  return;
}

} // namespace rc
