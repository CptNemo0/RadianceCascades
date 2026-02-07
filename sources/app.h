#ifndef RC_APP_H_
#define RC_APP_H_

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <memory>
#include <utility>

namespace rc {

class Ui;
class Renderer;

class App {
  public:
    static App& Instance() {
      static App instance;
      return instance;
    }

    App(const App&) = delete;
    void operator=(const App&) = delete;

    App(App&&) = delete;
    void operator=(App&&) = delete;

    ~App();

    void ProcessInput();

    bool ShouldRun();

    std::pair<float, float> GetCursorPosition();

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

  private:
    App();
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Ui> ui_;
};

} // namespace rc

#endif //! RC_APP_H_
