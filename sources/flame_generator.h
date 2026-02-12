#ifndef RC_FLAME_GENERATOR_H_
#define RC_FLAME_GENERATOR_H_

#include "aliasing.h"
#include "glm/glm.hpp"

#include <array>
#include <chrono>
#include <memory>

#include "app.h"
#include "constants.h"
#include "glm/fwd.hpp"
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

  private:
    friend class Ui;

    std::array<glm::vec2, gMaxFlameCount> positions_;

    u64 next_index_ = 0;
    float flame_size_ = 10.0f;
    float flame_speed_ = 1.0f;

    i64 time_between_fire_placements_ = 500;
    time_point last_placement_time_{};

    bool first_{true};
    bool eraser_{false};
    bool turned_on_{false};
    bool register_{false};

    RenderTarget render_target_fire_{rc::gScreenWidth, rc::gScreenHeight};
    RenderTarget render_target_combined_1_{rc::gScreenWidth, rc::gScreenHeight};
    RenderTarget render_target_combined_2_{rc::gScreenWidth, rc::gScreenHeight};
    std::array<RenderTarget*, 2> combined_rts_;

    std::unique_ptr<Texture> noise_texture_;

    ScopedObservation<FlameGenerator, App> app_observation_;
};

} // namespace rc

#endif // !RC_FLAME_GENERATOR_H_
