#include "ui.h"

#include <array>
#include <cstddef>
#include <print>

#include "constants.h"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <string>

#include "aliasing.h"
#include "renderer.h"

namespace {

// Imgui wants this old format...
const char* gModeStrings[] = {"Global illumination", "Radiance cascades"};

} // namespace

namespace rc {

Ui::Ui(GLFWwindow* window, Renderer* renderer)
  : window_(window), renderer_(renderer) {
  // 1. Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430");

  if (renderer_) {
    brush_color_ = renderer->canvas_->brush_color_;
    brush_size_ = renderer->canvas_->brush_radius_;
    last_pipeline_step_ = renderer_->gi_pipeline_.size() - 1;
  }
}

Ui::~Ui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Ui::Render() {
  // --- Start the Dear ImGui frame ---
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Renderer Settings");

  ImGui::Text(
    "DRAW with RIGHT mouse button.\nCHANGE SETTINGS with LEFT mouse button.");
  ImGui::Separator();

  ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)",
              1000.0 / static_cast<f64>(ImGui::GetIO().Framerate),
              static_cast<f64>(ImGui::GetIO().Framerate));
  ImGui::Separator();

  if (ImGui::ColorEdit3("Brush color",
                        glm::value_ptr(renderer_->canvas_->brush_color_))) {
    renderer_->canvas_->first_ = true;
  }

  if (ImGui::SliderFloat("Brush size", &renderer_->canvas_->brush_radius_, 0.0f,
                         rc::gMaxBrushRadius)) {
    renderer_->canvas_->first_ = true;
  }

  if (ImGui::Checkbox("Eraser", &renderer_->canvas()->eraser_)) {
    renderer_->canvas()->first_ = true;
  }

  int current_mode_index = static_cast<int>(renderer_->mode_);
  if (ImGui::Combo("Pipeline Mode", &current_mode_index, gModeStrings,
                   static_cast<size_t>(Renderer::Mode::kModeNumber))) {
    renderer_->mode_ = static_cast<Renderer::Mode>(current_mode_index);
  }

  switch (renderer_->mode_) {
  case Renderer::Mode::kGi:
    if (ImGui::SliderInt("Stage to render", &renderer_->stage_to_render_, 0,
                         renderer_->gi_pipeline_.size() - 1)) {
    }

    if (ImGui::SliderInt("Step count",
                         &renderer_->global_illumination_params_.step_count, 1,
                         64)) {
      renderer_->global_illumination_params_.dirty = true;
    }

    if (ImGui::SliderFloat(
          "Proximity threshold",
          &renderer_->global_illumination_params_.proximity_epsilon, 0.00001f,
          0.05f, "%.5f")) {
      renderer_->global_illumination_params_.dirty = true;
    }

    if (ImGui::SliderInt("Ray count",
                         &renderer_->global_illumination_params_.ray_count, 1,
                         64)) {
      renderer_->global_illumination_params_.dirty = true;
      renderer_->global_illumination_params_.one_over_ray_count =
        1.0f / renderer_->global_illumination_params_.ray_count;
      renderer_->global_illumination_params_.angle_step =
        static_cast<float>(std::numbers::pi) * 2.0f *
        renderer_->global_illumination_params_.one_over_ray_count;
    }

    if (ImGui::SliderFloat("Noise amount",
                           &renderer_->global_illumination_params_.noise_amount,
                           0.0f, 1.0f, "%.2f")) {
      renderer_->global_illumination_params_.dirty = true;
    }
    break;

  case Renderer::Mode::kRc:
    if (ImGui::SliderInt("Stage to render", &renderer_->stage_to_render_, 0,
                         renderer_->cascades_pipeline_.size() - 1)) {
    }
    break;
  case Renderer::Mode::kModeNumber:
    break;
  }

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace rc
