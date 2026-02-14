#ifndef RC_APP_H_
#define RC_APP_H_

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include "glm/fwd.hpp"
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>

#include "aliasing.h"

namespace rc {

class MeasurementManager;
class Renderer;
class Ui;

// The Big Boss class.
// It stores the renderers, initializes third party libraries and loads OpenGL.
// It's responsible for the general flow of the program and interfacing with the
// window manager.
class App {
  public:
    class Observer {
      public:
        virtual void GetMousePositionOnRMB(const glm::vec2& point) = 0;
        virtual ~Observer() = default;
    };

    static App& Instance() {
      static App instance;
      return instance;
    }

    App(const App&) = delete;
    void operator=(const App&) = delete;

    App(App&&) = delete;
    void operator=(App&&) = delete;

    ~App();

    void Start();

    void StartFrame();

    bool ShouldRun();

    glm::vec2 GetCursorPosition();

    bool RMBPressed();

    void EndFrame();

    float GetTime();

    GLFWwindow* window_{};

    Renderer* scene_renderer() {
      return renderer_.get();
    }

    Ui* ui() {
      return ui_.get();
    }

    MeasurementManager* measurement_manager() {
      return measurement_manager_.get();
    }

    void AddObserver(Observer* observer);

    void RemoveObserver(Observer* observer);

    void RegisterMousePosition();

    void StartMeasuring();

    void StopMeasuring();

  private:
    friend class Ui;

    App();

    std::unique_ptr<MeasurementManager> measurement_manager_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Ui> ui_;

    u64 frames_measured_{};
    bool is_measuring_;

    std::vector<Observer*> observers_;
};

} // namespace rc

#endif //! RC_APP_H_
