#include "flame_generator.h"

#include "constants.h"
#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"

#include <chrono>
#include <utility>

#include "app.h"
#include "shader.h"
#include "shader_manager.h"
#include "surface.h"
#include "utility.h"

namespace rc {

FlameGenerator::FlameGenerator()
  : noise_texture_(GetFBMTexture(512, 512, 6.0f, 5.0f)),
    app_observation_(this) {
  app_observation_.Observe(&(App::Instance()));

  for (auto& position : positions_) {
    position = glm::vec2(-1.0, 0.0);
  }

  combined_rts_[0] = &render_target_combined_1_;
  combined_rts_[1] = &render_target_combined_2_;
}

void FlameGenerator::GetMousePositionOnRMB(const glm::vec2& position) {
  if (!register_) {
    return;
  }

  if (eraser_) {
    for (auto& p : positions_) {
      if (glm::distance(p, position) <= flame_size_) {
        p.x = -1.0f;
      }
    }
    return;
  }

  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_placement_time_)
        .count() >= time_between_fire_placements_) {
    last_placement_time_ = now;
    positions_[(next_index_++) % positions_.size()] = position;
  }
}

void FlameGenerator::RenderFlames() {
  if (!turned_on_) {
    combined_rts_[0]->Bind();
    combined_rts_[0]->Clear();
    return;
  }

  const Shader* flame_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kFlame);
  render_target_fire_.Bind();
  render_target_fire_.Clear();
  noise_texture_->BindTexture(GL_TEXTURE0);
  flame_shader->setFloat("time", App::Instance().GetTime());
  flame_shader->setFloat("percentage",
                         (flame_size_ / gMaxBrushRadius) * gMaxFlameSize);
  flame_shader->setFloat("speed", flame_speed_);
  Surface::Instnace().Draw();

  const Shader* combine_shader =
    ShaderManager::Instance().Use(ShaderManager::ShaderType::kOverlay);
  combine_shader->setInt("texture_1", 0);
  combine_shader->setInt("texture_2", 1);
  combine_shader->setVec2("offset_2", glm::vec2(0.0));
  render_target_fire_.BindTexture(GL_TEXTURE0);

  for (auto* rt : combined_rts_) {
    rt->Bind();
    rt->Clear();
  }

  const glm::vec2 reverse_resolution =
    1.0f / glm::vec2(gScreenWidth, gScreenHeight);

  for (const glm::vec2& position : positions_) {
    if (position.x < 0.0f) {
      continue;
    }

    combine_shader->setVec2(
      "offset_1", (position * reverse_resolution - glm::vec2(0.5f, 0.5f)) *
                      glm::vec2(-1.0f, 1.0f) +
                    glm::vec2(0.0, -0.8f * (flame_size_ / gMaxBrushRadius) *
                                     gMaxFlameSize * 0.5f));

    combined_rts_[0]->BindTexture(GL_TEXTURE1);
    combined_rts_[1]->Bind();
    combined_rts_[1]->Clear();

    Surface::Instnace().Draw();

    std::swap(combined_rts_[0], combined_rts_[1]);
  }
}

} // namespace rc
