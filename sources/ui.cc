#include "ui.h"

#include <cstddef>

#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "aliasing.h"
#include "constants.h"
#include "renderer.h"

namespace {

// Imgui wants this old format...
const char* gModeStrings[] = {"Global illumination", "Radiance cascades"};

} // namespace

namespace rc {

Ui::Ui(GLFWwindow* window, Renderer* renderer)
  : renderer_(renderer), flame_generator_(renderer->flame_generator_.get()) {
  // 1. Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 430");
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
    flame_generator_->flame_size_ = renderer_->canvas_->brush_radius_;
  }

  if (ImGui::Checkbox("Draw canvas", &renderer_->canvas()->register_)) {
    flame_generator_->register_ = false;
  }

  if (ImGui::Checkbox("Draw flames", &flame_generator_->register_)) {
    flame_generator_->turned_on_ = true;
    renderer_->canvas()->register_ = false;
  }

  if (ImGui::Checkbox("Display Flames", &flame_generator_->turned_on_)) {
    renderer_->canvas()->first_ = true;
  }

  if (flame_generator_->turned_on_) {
    ImGui::SliderFloat("Flame speed", &flame_generator_->flame_speed_, 0.0f,
                       2.0f);
  }

  if (ImGui::Checkbox("Eraser", &renderer_->canvas()->eraser_)) {
    renderer_->canvas()->first_ = true;
    flame_generator_->eraser_ = renderer_->canvas()->eraser_;
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

    if (ImGui::SliderInt("Step count", &renderer_->cascades_params_.step_count,
                         1, 64)) {
      renderer_->cascades_params_.dirty = true;
    }

    if (ImGui::SliderFloat("Proximity threshold",
                           &renderer_->cascades_params_.proximity_epsilon,
                           0.00001f, 0.05f, "%.5f")) {
      renderer_->global_illumination_params_.dirty = true;
    }

    if (ImGui::SliderInt("Base rays count",
                         &renderer_->cascades_params_.base_ray_count, 4, 64)) {
      renderer_->cascades_params_.dirty = true;
    }

    if (ImGui::SliderInt("Cascade count",
                         &renderer_->cascades_params_.cascade_count, 1, 16)) {
      renderer_->cascades_params_.dirty = true;
    }

    if (ImGui::SliderFloat("Ray overlap", &renderer_->cascades_params_.overlap,
                           0.0f, 2.0f, "%.2f")) {
      renderer_->cascades_params_.dirty = true;
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
