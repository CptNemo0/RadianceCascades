#ifndef RC_CANVAS_H_
#define RC_CANVAS_H_

#include <array>
#include <chrono>

#include "glm/fwd.hpp"
#include "glm/glm.hpp"

#include "aliasing.h"
#include "app.h"
#include "constants.h"
#include "render_target.h"
#include "scoped_observation.h"

namespace rc {

class Ui;

// The main drawing class.
class Canvas : public App::Observer {
  public:
    Canvas(u64 height, u64 width, u64 brush_radius = 10,
           glm::vec3 brush_color = {1.0f, 1.0f, 1.0f});

    u64 height() const {
      return height_;
    }

    u64 width() const {
      return width_;
    }

    u64 brush_radius() const {
      return brush_radius_;
    }

    virtual void GetMousePositionOnRMB(const glm::vec2& point) override;

    void Draw();

    void BindTexture(int texture_slot) {
      render_targets_[0]->BindTexture(texture_slot);
    }

    void set_brush_color(glm::vec3 brush_color) {
      first_ = true;
      brush_color_ = brush_color;
    }

    void set_brush_radius(float radius) {
      first_ = true;
      brush_radius_ = radius;
    }

  private:
    friend class Ui;

    u64 height_;
    u64 width_;

    float brush_radius_;
    glm::vec3 brush_color_;

    bool first_{true};
    bool eraser_{false};
    bool register_{true};
    std::chrono::high_resolution_clock::time_point last_draw_;

    glm::vec2 previous_position_;
    glm::vec2 selected_position_;

    rc::RenderTarget render_target_canvas_1{rc::gScreenWidth,
                                            rc::gScreenHeight};
    rc::RenderTarget render_target_canvas_2{rc::gScreenWidth,
                                            rc::gScreenHeight};
    std::array<rc::RenderTarget*, 2> render_targets_{&render_target_canvas_1,
                                                     &render_target_canvas_2};

    ScopedObservation<Canvas, App> app_observation_;
};

} // namespace rc

#endif // !RC_CANVAS_H_
