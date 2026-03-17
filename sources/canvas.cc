#include "canvas.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <print>

#include "app.h"
#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"

#include "aliasing.h"
#include "constants.h"
#include "glm/geometric.hpp"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"

namespace rc {

Canvas::Canvas(u64 height, u64 width, u64 brush_radius, glm::vec3 brush_color)
  : height_(height), width_(width),
    brush_radius_(std::clamp(brush_radius, 0uz, gMaxBrushRadius)),
    brush_color_(brush_color), app_observation_(this) {
  app_observation_.Observe(&(App::Instance()));

  const rc::Shader* canvas_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kCanvas);
  canvas_shader->SetFloat("width", static_cast<float>(width_));
  canvas_shader->SetFloat("height", static_cast<float>(height_));
  canvas_shader->SetFloat("brush_radius", brush_radius_ * brush_radius_ *
                                            gBrushScale * gBrushScale);
  canvas_shader->SetVec3("brush_color", brush_color_);
  canvas_shader->SetBool("eraser", eraser_);

  cached_position_location_ =
    glGetUniformLocation(canvas_shader->id_, "position");

  for (int i = 0; i < 50; ++i) {
    predefined_points_.push_back(RandomPoint{});
  }
}

void Canvas::GetMousePositionOnRMB(const glm::vec2& position) {
  if (!register_) {
    return;
  }
  float x = position.x;
  float y = position.y;

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

void Canvas::DrawPoint(const glm::vec2 sp) {
  ShaderManager::Instance().Use(ShaderManager::ShaderType::kCanvas);
  render_targets_[0]->Bind();
  render_targets_[0]->Clear();
  render_targets_[1]->BindTexture(GL_TEXTURE0);
  glUniform2f(cached_position_location_, sp.x, sp.y);
  Surface::Instance().Draw();
  std::swap(render_targets_[0], render_targets_[1]);
};

void Canvas::DrawPredefined() {
  for (auto& point : predefined_points_) {
    const Shader* shader =
      ShaderManager::Instance().Use(ShaderManager::ShaderType::kCanvas);
    shader->SetVec3("brush_color", point.color);
    shader->SetFloat("brush_radius",
                     point.radius * point.radius * gBrushScale * gBrushScale);
    DrawPoint(point.position);
  };
  first_ = true;
};

void Canvas::Draw() {
  if (first_) {
    return;
  }

  const glm::vec2 sp =
    selected_position_ * glm::vec2(gOneOverWidth, gOneOverHeight);

  DrawPoint(sp);

  // ShaderManager::Instance().Use(ShaderManager::ShaderType::kCanvas);
  // render_targets_[0]->Bind();
  // render_targets_[0]->Clear();
  // render_targets_[1]->BindTexture(GL_TEXTURE0);
  // glUniform2f(cached_position_location_, sp.x, sp.y);
  // Surface::Instnace().Draw();
  // std::swap(render_targets_[0], render_targets_[1]);

  /*
   *  ShaderManager::Instance()
     .Use(ShaderManager::ShaderType::kCanvas)
     ->setVec3("brush_color", renderer_->canvas_->brush_color_);
   */
}

} // namespace rc
