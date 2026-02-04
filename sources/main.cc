#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <exception>
#include <print>

#include "app.h"

int main() {
  try {
    rc::App& app = rc::App::Instance();

    while (app.ShouldRun()) {
      app.ProcessInput();
      if (app.LMBPressed()) {
        const auto [x, y] = app.GetCursorPosition();
        app.renderer()->canvas().RegisterPoint(static_cast<float>(x),
                                               static_cast<float>(y));
      }
      app.renderer()->Render();
      app.EndFrame();
    }
  } catch (std::exception& e) {
    std::print("{}\n", e.what());
  }

  return 0;
}
