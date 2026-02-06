#include "ui.h"

#include <print>

#include "constants.h"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "aliasing.h"
#include "renderer.h"

namespace rc {

Ui::Ui(GLFWwindow* window, Renderer* renderer)
  : window_(window), renderer_(renderer) {
  // 1. Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
    ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  // 2. Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // 3. Setup Platform/Renderer backends
  // Install_Callbacks=true will install GLFW callbacks and chain to existing
  // ones.
  ImGui_ImplGlfw_InitForOpenGL(window, true);

  // Assuming OpenGL 3.3+. Adjust version string "#version 330" if needed.
  ImGui_ImplOpenGL3_Init("#version 430");

  if (renderer_) {
    brush_color_ = renderer->canvas_->brush_color_;
    brush_size_ = renderer->canvas_->brush_radius_;
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

  ImGui::Text("Controls");
  if (ImGui::ColorEdit3("Brush color", glm::value_ptr(brush_color_))) {
    renderer_->canvas_->set_brush_color(brush_color_);
  }

  if (ImGui::SliderFloat("Brush size", &brush_size_, 0.0f,
                         rc::gMaxBrushRadius)) {
    renderer_->canvas_->set_brush_radius(brush_size_);
  }

  const char* mode_names[] = {"Global Illumination", "Radiance Cascades"};
  int current_mode_index = static_cast<int>(renderer_->mode_);
  if (ImGui::Combo("Pipeline Mode", &current_mode_index, mode_names,
                   IM_ARRAYSIZE(mode_names))) {
    renderer_->mode_ = static_cast<Renderer::Mode>(current_mode_index);
  }

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace rc
