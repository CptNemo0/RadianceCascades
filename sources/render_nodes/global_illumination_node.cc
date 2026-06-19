#include "render_nodes/global_illumination_node.h"

#include "glad/include/glad/glad.h"

#include <array>
#include <initializer_list>
#include <memory>
#include <string_view>
#include <utility>

#include "aliasing.h"
#include "app.h"
#include "async_image_writer.h"
#include "constants.h"
#include "image_write.h"
#include "render_nodes/render_node.h"
#include "render_target.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "timed_scope.h"
#include "utility.h"

namespace rc {

namespace {

std::array<u8, gScreenHeight * gScreenWidth * 4> pixels;

}

GlobalIlluminationNode::GlobalIlluminationNode(
    std::string_view name,
    GlobalIlluminationNode::Parameters& params,
    std::initializer_list<RenderNode*> inputs)
    : RenderNode(name, inputs),
      parameters_(params),
      gi_render_target_(std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                       rc::gScreenHeight,
                                                       GL_RGBA8,
                                                       GL_RGBA,
                                                       GL_UNSIGNED_BYTE)),
      previous_frame_(std::make_unique<RenderTarget>(rc::gScreenWidth,
                                                     rc::gScreenHeight,
                                                     GL_RGBA8,
                                                     GL_RGBA,
                                                     GL_UNSIGNED_BYTE)),
      noise_texture_(rc::GetNoiseTexture(rc::gScreenWidth, rc::gScreenHeight)) {
  const Shader* shader{ShaderManager::Instance().Use(ShaderType::kGi)};
  // I know, this is not the perfect way to do this. But currently writing
  // parsing logic for this is the last thing on my mind.
  shader->SetInt("color_texture", 0);
  shader->SetInt("sdf_texture", 1);
  shader->SetInt("previous_frame", 2);
  shader->SetInt("noise_texture", 3);
}

void GlobalIlluminationNode::Forward() {
  TimedScope timed_scope{ShouldMeasure() ? this : nullptr};
  // Not a very pretty hack.
  float time{App::Instance().GetTime() * 1000.f - time_normalizer_};
  if (time > 1000.0f) {
    time_normalizer_ += 1000.0f;
  }

  const Shader* shader{ShaderManager::Instance().Use(ShaderType::kGi)};
  BindInputs();
  UpdateUniforms();
  shader->SetFloat("time", App::Instance().GetTime());
  gi_render_target_->Bind();
  gi_render_target_->Clear();
  previous_frame_->BindTexture(GL_TEXTURE0 + inputs_.size() + 0);
  noise_texture_->BindTexture(GL_TEXTURE0 + inputs_.size() + 1);
  Surface::Instance().Draw();

  ShaderManager::Instance().Use(ShaderType::kSurface);
  previous_frame_->Bind();
  previous_frame_->Clear();
  gi_render_target_->BindTexture(GL_TEXTURE0);
  Surface::Instance().Draw();

  if (App::Instance().IsMeasuring()) {
    previous_frame_->Bind();
    if (App::Instance().ShouldSaveFrame()) {
      CreateImageOutputDir();
      ReadFramebufferRgba(pixels);
      App::Instance().async_writer()->PushWork(
          std::move(pixels), App::Instance().MeasuredFrameIndex());
    }
  }
}

void GlobalIlluminationNode::UpdateUniforms() {
  if (!parameters_.dirty) {
    return;
  }
  const Shader* shader{ShaderManager::Instance().Use(ShaderType::kGi)};
  shader->SetInt("step_count", parameters_.step_count);
  shader->SetFloat("proximity_epsilon", parameters_.proximity_epsilon);
  shader->SetFloat("one_over_ray_count", parameters_.one_over_ray_count);
  shader->SetFloat("angle_step", parameters_.angle_step);
  shader->SetFloat("noise", parameters_.noise_amount);
  parameters_.dirty = false;
}

}  // namespace rc
