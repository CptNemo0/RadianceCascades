#ifndef RC_FLAME_GENERATOR_H_
#define RC_FLAME_GENERATOR_H_

#include "glad/include/glad/glad.h"

#include <array>
#include <chrono>
#include <memory>

#include "aliasing.h"
#include "app.h"
#include "constants.h"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "render_target.h"
#include "scoped_observation.h"
#include "texture.h"

namespace rc {

class Ui;

class FlameGenerator : public App::Observer {
 public:
  using time_point = std::chrono::steady_clock::time_point;

  FlameGenerator();

  virtual void GetMousePositionOnRMB(const glm::vec2& point) override;

  const std::array<glm::vec2, gMaxFlameCount>& positions() const {
    return positions_;
  }

  void RenderFlames();

  void BindTexture(int texture_slot) {
    combined_rts_[0]->BindTexture(texture_slot);
  }

  void Clear() {
    first_ = true;
    render_target_combined_1_.Bind();
    render_target_combined_1_.Clear();
    render_target_combined_2_.Bind();
    render_target_combined_2_.Clear();
    RenderTarget::BindDefault();
  }

  void DrawPredefined() {
    for (glm::vec2& pos : positions_) {
      pos.x = -1.0f;
    }

    positions_[0] = {gScreenWidth / 2.0f, gScreenHeight / 2.0f};
    flame_size_ = 40;
    next_index_ = 1;
    first_ = true;
  };

 private:
  friend class Ui;

  std::array<glm::vec2, gMaxFlameCount> positions_;

  RenderTarget render_target_fire_{rc::gScreenWidth, rc::gScreenHeight,
                                   GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
  RenderTarget render_target_combined_1_{rc::gScreenWidth, rc::gScreenHeight,
                                         GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
  RenderTarget render_target_combined_2_{rc::gScreenWidth, rc::gScreenHeight,
                                         GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};

  std::array<RenderTarget*, 2> combined_rts_;

  ScopedObservation<FlameGenerator, App> app_observation_;

  u64 next_index_{0};

  i64 time_between_fire_placements_{500};
  time_point last_placement_time_{};

  float flame_size_{10.0f};
  float flame_speed_{0.2f};

  std::unique_ptr<Texture> noise_texture_;

  bool first_{true};
  bool eraser_{false};
  bool turned_on_{false};
  bool register_{false};
};

}  // namespace rc

#endif  // !RC_FLAME_GENERATOR_H_
