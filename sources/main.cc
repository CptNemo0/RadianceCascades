#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <concepts>
#include <cstdlib>
#include <ctime>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <exception>
#include <print>
#include <string>

#include "aliasing.h"
#include "app.h"
#include "canvas.h"
#include "constants.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "utility.h"

namespace rc {
class Renderer {};

class RenderNode {
  public:
    RenderNode() = default;
    virtual ~RenderNode() = default;

    // Performs node's rendering routine.
    virtual void Forward() = 0;

    // Binds output texture to the provided texture slot.
    virtual void BindOutput(int texture_slot) const = 0;

    void BindInputs() {
      for (auto i{0uz}; i < inputs_.size(); ++i) {
        if (!inputs_[i]) {
          throw std::runtime_error(
            "BindPreviousOutput(): An input render target was a nullptr\n");
        }

        inputs_[i]->BindOutput(GL_TEXTURE0 + i);
      }
    }

  protected:
    RenderNode(std::initializer_list<RenderNode*> inputs) : inputs_(inputs) {};
    std::vector<RenderNode*> inputs_{};
};

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

} // namespace rc

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

    std::vector<rc::RenderNode*> render_nodes{canvas_node.get(),
                                              uv_colorspace_node.get(),
                                              jfa_node.get(), sdf_node.get()};

    rc::RenderTarget render_target_jfa_1{rc::gScreenWidth, rc::gScreenHeight};
    rc::RenderTarget render_target_jfa_2{rc::gScreenWidth, rc::gScreenHeight};
    std::array<rc::RenderTarget*, 2> jfa_render_targets{&render_target_jfa_1,
                                                        &render_target_jfa_2};

    rc::RenderTarget render_target_sdf{rc::gScreenWidth, rc::gScreenHeight};

    rc::RenderTarget render_target_gi{rc::gScreenWidth, rc::gScreenHeight};

    rc::RenderTarget render_target_rc_1{rc::gScreenWidth, rc::gScreenHeight};
    rc::RenderTarget render_target_rc_2{rc::gScreenWidth, rc::gScreenHeight};
    std::array<rc::RenderTarget*, 2> rc_render_targets{&render_target_rc_1,
                                                       &render_target_rc_2};

    rc::RenderTarget render_target_previous_frame{rc::gScreenWidth,
                                                  rc::gScreenHeight};

    auto noise_texture =
      rc::GetNoiseTexture(rc::gScreenWidth, rc::gScreenHeight);

    const f32 interval_overlap = 0.001f;

    while (app.ShouldRun()) {
      app.ProcessInput();
      if (app.LMBPressed()) {
        const auto [x, y] = app.GetCursorPosition();
        canvas.RegisterPoint(static_cast<float>(x), static_cast<float>(y));
      }

      for (auto* node : render_nodes) {
        std::print("Forward\n");
        node->Forward();
      }

/*
      canvas.Draw();

      // Copy canvas to jfa_1
      shader_manager.Use(ShaderType::kSurface);
      render_target_jfa_1.Bind();
      render_target_jfa_1.Clear();
      canvas.BindTexture(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();

      // Create uv color space representation of canvas in jfa_2
      shader_manager.Use(ShaderType::kUvColorspace);
      render_target_jfa_2.Bind();
      render_target_jfa_2.Clear();
      render_target_jfa_1.BindTexture(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();

      const rc::Shader* shader_jfa = shader_manager.Use(ShaderType::kJfa);
      shader_jfa->setFloat("width", rc::gScreenWidth);
      shader_jfa->setFloat("height", rc::gScreenHeight);
      shader_jfa->setFloat("one_over_width", rc::gOneOverWidth);
      shader_jfa->setFloat("one_over_height", rc::gOneOverHeight);

      // JFA
      // "Step length starts at 1 and than doubles until  N/2 for the scatter
      // approach." ~ JFA article.
      // Inverse should happen in the case of gather approach. From N/2 to 1.
      rc::u32 jfa_swaps = 0;
      for (auto i{0uz}; i < rc::gJfaSteps; ++i) {
        // -1 to start from N / 2
        shader_jfa->setInt("step_size", std::pow(2, rc::gJfaSteps - i - 1));

        jfa_render_targets[jfa_swaps % 2]->Bind();
        jfa_render_targets[jfa_swaps % 2]->Clear();
        jfa_render_targets[(jfa_swaps + 1) % 2]->BindTexture(GL_TEXTURE0);
        rc::Surface::Instnace().Draw();

        jfa_swaps++;
      }

      shader_manager.Use(ShaderType::kSdf);
      render_target_sdf.Bind();
      render_target_sdf.Clear();
      jfa_render_targets[(jfa_swaps + 1) % 2]->BindTexture(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();
*/
#pragma region GI
      // const rc::Shader* shader_gi = rc::ShaderManager::Instance().GetShader(
      //   rc::ShaderManager::ShaderType::kGi);
      // shader_gi->use();
      // shader_gi->setInt("step_count", 1024);
      // shader_gi->setFloat("proximity_epsilon", 0.0001f);

      // shader_gi->setInt("ray_count", static_cast<float>(ray_count));
      // shader_gi->setFloat("one_over_ray_count", 1.0f / ray_count);
      // shader_gi->setFloat("angle_step", 2 * 3.14159f / ray_count);

      // shader_gi->setFloat("width", rc::gScreenWidth);
      // shader_gi->setFloat("height", rc::gScreenHeight);

      // shader_gi->setInt("color_texture", 0);
      // shader_gi->setInt("sdf_texture", 1);
      // shader_gi->setInt("previous_frame", 2);
      // shader_gi->setInt("noise_texture", 3);

      // shader_gi->setFloat("time", app.GetTime() * 1000.f);

      // render_target_gi.Bind();
      // render_target_gi.Clear();
      // canvas.BindTexture(GL_TEXTURE0);
      // render_target_sdf.BindTexture(GL_TEXTURE1);
      // render_target_previous_frame.BindTexture(GL_TEXTURE2);
      // noise_texture->BindTexture(GL_TEXTURE3);
      // rc::Surface::Instnace().Draw();
#pragma endregion GI

      auto diagonal = std::sqrt(rc::gScreenWidth * rc::gScreenWidth +
                                rc::gScreenHeight * rc::gScreenHeight);

      auto cascade_count = static_cast<int>(
        std::ceil(std::log(diagonal) / std::log(rc::gBaseRayCount)));
      const rc::Shader* shader_rc = shader_manager.Use(ShaderType::kRc);
      shader_rc->setVec2("resolution",
                         glm::vec2(rc::gScreenWidth, rc::gScreenHeight));
      shader_rc->setInt("sceneTexture", 0);
      shader_rc->setInt("distanceTexture", 1);
      shader_rc->setInt("lastTexture", 2);
      shader_rc->setFloat("base", rc::gBaseRayCount);
      shader_rc->setFloat("cascadeCount", cascade_count);
      shader_rc->setFloat("srgb", 2.1f);

      canvas_node->BindOutput(GL_TEXTURE0);
      sdf_node->BindOutput(GL_TEXTURE1);

      // canvas.BindTexture(GL_TEXTURE0);
      // render_target_sdf.BindTexture(GL_TEXTURE1);

      // Last frame is second in the rc_render_targets array.
      for (int i{cascade_count - 1}; i > -1; --i) {
        shader_rc->setFloat("cascadeIndex", i);
        shader_rc->setBool("lastIndex", i == 0);
        float scale = std::powf(2, i + 1);
        shader_rc->setVec2("resolution",
                           glm::vec2(rc::gScreenWidth, rc::gScreenHeight));
        rc_render_targets[0]->Bind();
        rc_render_targets[0]->ClearDefault();
        rc_render_targets[1]->BindTexture(GL_TEXTURE2);

        rc::Surface::Instnace().Draw();
        std::swap(rc_render_targets[0], rc_render_targets[1]);
      }

      // Copy to previous frame
      // rc::ShaderManager::Instance().GetShader(
      //   rc::ShaderManager::ShaderType::kSurface);
      // render_target_previous_frame.Bind();
      // render_target_previous_frame.Clear();
      // render_target_rc_1.BindTexture(GL_TEXTURE0);
      // rc::Surface::Instnace().Draw();

      // Draw to screen
      shader_manager.Use(ShaderType::kSurface);
      rc::RenderTarget::BindDefault();
      rc::RenderTarget::ClearDefault();
      rc_render_targets[1]->BindTexture(GL_TEXTURE0);
      rc::Surface::Instnace().Draw();

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
