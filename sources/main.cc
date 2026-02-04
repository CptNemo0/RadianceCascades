#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE

#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"

#include <exception>
#include <format>
#include <initializer_list>
#include <memory>
#include <print>
#include <vector>

#include "aliasing.h"
#include "app.h"
#include "canvas.h"
#include "constants.h"
#include "render_nodes/canvas_node.h"
#include "render_nodes/copy_node.h"
#include "render_nodes/global_illumination_node.h"
#include "render_nodes/jfa_node.h"
#include "render_nodes/radiance_cascades_node.h"
#include "render_nodes/render_node.h"
#include "shader_manager.h"

using rc::i8, rc::i32, rc::i64, rc::u8, rc::u32, rc::u64, rc::f32;
using ShaderType = rc::ShaderManager::ShaderType;

int main() {
  try {
    rc::App& app = rc::App::Instance();
    rc::ShaderManager& shader_manager = rc::ShaderManager::Instance();
    shader_manager.LoadShaders();

    rc::Canvas canvas{rc::gScreenHeight, rc::gScreenHeight, 10,
                      glm::vec3(1.0f, 1.0f, 1.0f)};

    std::unique_ptr<rc::CanvasNode> canvas_node =
      std::make_unique<rc::CanvasNode>(canvas);

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

    std::vector<rc::RenderNode*> render_nodes{
      canvas_node.get(), uv_colorspace_node.get(), jfa_node.get(),
      sdf_node.get(), rc_node.get()};

    std::unique_ptr<rc::CopyNode> to_screen_node =
      std::make_unique<rc::CopyNode>(
        std::initializer_list<rc::RenderNode*>{render_nodes.back()}, true);
    render_nodes.push_back(to_screen_node.get());

    while (app.ShouldRun()) {
      app.ProcessInput();
      if (app.LMBPressed()) {
        const auto [x, y] = app.GetCursorPosition();
        canvas.RegisterPoint(static_cast<float>(x), static_cast<float>(y));
      }

      for (auto* node : render_nodes) {
        node->Forward();
      }

      app.EndFrame();

      if (glfwGetKey(rc::App::Instance().window_, GLFW_KEY_W)) {
        canvas.set_brush_color({1.0, 1.0, 1.0});
      }

      if (glfwGetKey(rc::App::Instance().window_, GLFW_KEY_B)) {
        canvas.set_brush_color({0.0, 0.0, 0.0});
      }

      if (glfwGetKey(rc::App::Instance().window_, GLFW_KEY_R)) {
        canvas.set_brush_color({1.0, 0.0, 0.0});
      }

      if (glfwGetKey(rc::App::Instance().window_, GLFW_KEY_G)) {
        canvas.set_brush_color({0.0, 1.0, 0.0});
      }

      if (glfwGetKey(rc::App::Instance().window_, GLFW_KEY_T)) {
        canvas.set_brush_color({0.0, 0.0, 1.0});
      }
    }
  } catch (std::exception& e) {
    std::print("{}\n", e.what());
  }

  return 0;
}
