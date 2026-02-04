#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include "imgui/imgui.h"
#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <array>
#include <cmath>
#include <concepts>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <numbers>
#include <print>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "aliasing.h"
#include "app.h"
#include "canvas.h"
#include "constants.h"
#include "render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "texture.h"
#include "utility.h"

using rc::i8, rc::i32, rc::i64, rc::u8, rc::u32, rc::u64, rc::f32;
using ShaderType = rc::ShaderManager::ShaderType;

namespace rc {

class CopyNode : public RenderNode {
  public:
    explicit CopyNode(ShaderManager::ShaderType copy_shader,
                      std::initializer_list<RenderNode*> inputs)
      : RenderNode(inputs), copy_shader_(copy_shader),
        output_(
          std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)) {
    }

    virtual void Forward() override {
      ShaderManager::Instance().Use(copy_shader_);
      if (output_) {
        output_->Bind();
        output_->Clear();
      } else {
        RenderTarget::BindDefault();
        RenderTarget::ClearDefault();
      }
      BindInputs();
      rc::Surface::Instnace().Draw();
    }

    virtual void BindOutput(int texture_slot) const override {
      output_->BindTexture(texture_slot);
    }

  private:
    ShaderManager::ShaderType copy_shader_ =
      ShaderManager::ShaderType::kSurface;
    std::unique_ptr<RenderTarget> output_{nullptr};
};

class CanvasNode : public RenderNode {
  public:
    explicit CanvasNode(Canvas& canvas) : RenderNode({}), canvas_(canvas) {
    }

    virtual void Forward() override {
      canvas_.Draw();
    }

    virtual void BindOutput(int texture_slot) const override {
      canvas_.BindTexture(texture_slot);
    }

  private:
    Canvas& canvas_;
};

class JfaNode : public RenderNode {
  public:
    explicit JfaNode(RenderNode* initializer_node)
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

    virtual void Forward() override {
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

    virtual void BindOutput(int texture_slot) const override {
      render_targets_[1]->BindTexture(texture_slot);
    }

  private:
    void Initialize() {
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

    rc::u32 jfa_swaps{0};
    std::unique_ptr<rc::RenderTarget> render_target_1;
    std::unique_ptr<rc::RenderTarget> render_target_2;
    std::array<rc::RenderTarget*, 2> render_targets_;
};

class GlobalIlluminationNode : public RenderNode {
  public:
    struct Parameters {
        bool dirty{true};
        i32 step_count = 128;
        f32 proximity_epsilon = 0.00001f;
        i32 ray_count = 32;
        f32 one_over_ray_count = 1.0f / static_cast<float>(ray_count);
        f32 angle_step =
          static_cast<float>(std::numbers::pi) * one_over_ray_count;
    };
    GlobalIlluminationNode(std::initializer_list<RenderNode*> inputs)
      : RenderNode(inputs), gi_render_target_(std::make_unique<RenderTarget>(
                              rc::gScreenWidth, rc::gScreenHeight)),
        previous_frame_(
          std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)),
        noise_texture_(
          rc::GetNoiseTexture(rc::gScreenWidth, rc::gScreenHeight)) {
      const Shader* shader =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
      shader->setInt("color_texture", 0);
      shader->setInt("sdf_texture", 1);
      shader->setInt("previous_frame", 2);
      shader->setInt("noise_texture", 3);
    }

    virtual void Forward() override {
      float time = App::Instance().GetTime() * 1000.f - time_normalizer;
      if (time > 1000.0f) {
        time_normalizer += 1000.0f;
      }

      const Shader* shader =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
      BindInputs();
      UpdateUniforms();
      shader->setFloat("time", time);
      gi_render_target_->Bind();
      gi_render_target_->Clear();
      previous_frame_->BindTexture(GL_TEXTURE0 + inputs_.size() + 0);
      noise_texture_->BindTexture(GL_TEXTURE0 + inputs_.size() + 1);
      rc::Surface::Instnace().Draw();

      rc::ShaderManager::Instance().Use(ShaderManager::ShaderType::kSurface);
      previous_frame_->Bind();
      previous_frame_->Clear();
      gi_render_target_->BindTexture(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();
    }

    virtual void BindOutput(int texture_slot) const override {
      previous_frame_->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms() {
      if (!parameters_.dirty) {
        return;
      }
      const Shader* shader =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kGi);
      shader->setInt("step_count", parameters_.step_count);
      shader->setFloat("proximity_epsilon", parameters_.proximity_epsilon);
      shader->setInt("ray_count", parameters_.ray_count);
      shader->setFloat("one_over_ray_count", parameters_.one_over_ray_count);
      shader->setFloat("angle_step", parameters_.angle_step);
      parameters_.dirty = false;
    }

    Parameters parameters_;
    std::unique_ptr<RenderTarget> gi_render_target_;
    std::unique_ptr<RenderTarget> previous_frame_;
    std::unique_ptr<Texture> noise_texture_;
    float time_normalizer = 0.0;
};

class RadianceCascadesNode : public RenderNode {
  public:
    struct Parameters {
        i32 base_ray_count = 16;
        i32 cascade_count = static_cast<int>(
          std::ceil(std::log(std::sqrt(rc::gScreenWidth * rc::gScreenWidth +
                                       rc::gScreenHeight * rc::gScreenHeight)) /
                    std::log(rc::gBaseRayCount)));
        f32 s_rgb = 2.1;
        bool dirty = true;
    };

    RadianceCascadesNode(std::initializer_list<RenderNode*> inputs)
      : RenderNode(inputs), render_target_1_(std::make_unique<RenderTarget>(
                              rc::gScreenWidth, rc::gScreenHeight)),
        render_target_2_(
          std::make_unique<RenderTarget>(rc::gScreenWidth, rc::gScreenHeight)) {
      const rc::Shader* shader_rc =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
      shader_rc->setVec2("resolution",
                         glm::vec2(rc::gScreenWidth, rc::gScreenHeight));
      shader_rc->setInt("sceneTexture", 0);
      shader_rc->setInt("distanceTexture", 1);
      shader_rc->setInt("lastTexture", 2);
      render_targets_[0] = render_target_1_.get();
      render_targets_[1] = render_target_2_.get();
    }

    virtual void Forward() override {
      const Shader* shader =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
      UpdateUniforms();
      BindInputs();

      // Last frame is second in the rc_render_targets array.
      for (int i{parameters_.cascade_count - 1}; i > -1; --i) {
        shader->setFloat("cascadeIndex", i);
        shader->setBool("lastIndex", i == 0);
        render_targets_[0]->Bind();
        render_targets_[0]->ClearDefault();
        render_targets_[1]->BindTexture(GL_TEXTURE2);
        rc::Surface::Instnace().Draw();
        std::swap(render_targets_[0], render_targets_[1]);
      }
    }

    virtual void BindOutput(int texture_slot) const override {
      render_targets_[1]->BindTexture(texture_slot);
    }

    Parameters& get_params() {
      return parameters_;
    }

  private:
    void UpdateUniforms() {
      if (!parameters_.dirty) {
        return;
      }
      const rc::Shader* shader_rc =
        ShaderManager::Instance().Use(ShaderManager::ShaderType::kRc);
      shader_rc->setFloat("base", parameters_.base_ray_count);
      shader_rc->setFloat("cascadeCount", parameters_.cascade_count);
      shader_rc->setFloat("srgb", parameters_.s_rgb);
      parameters_.dirty = false;
    }

    Parameters parameters_;
    std::unique_ptr<RenderTarget> render_target_1_;
    std::unique_ptr<RenderTarget> render_target_2_;
    std::array<RenderTarget*, 2> render_targets_;
};

} // namespace rc

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
      sdf_node.get(), gi_node.get()};

    while (app.ShouldRun()) {
      app.ProcessInput();
      if (app.LMBPressed()) {
        const auto [x, y] = app.GetCursorPosition();
        canvas.RegisterPoint(static_cast<float>(x), static_cast<float>(y));
      }

      for (auto* node : render_nodes) {
        node->Forward();
      }

      rc::ShaderManager::Instance().Use(ShaderType::kSurface);
      rc::RenderTarget::BindDefault();
      rc::RenderTarget::ClearDefault();
      render_nodes.back()->BindOutput(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
