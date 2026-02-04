#ifndef RC_APP_H_
#define RC_APP_H_

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <utility>

namespace rc {

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

    bool LMBPressed();

    void EndFrame();

    float GetTime();

    GLFWwindow* window_{};

  private:
    App();
};

} // namespace rc

#endif //! RC_APP_H_
