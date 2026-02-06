#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <exception>
#include <print>

#include "app.h"
#include "canvas.h"
#include "renderer.h"
#include "ui.h"

int main() {
  try {
    rc::App& app = rc::App::Instance();

    while (app.ShouldRun()) {
      app.ProcessInput();
      if (app.RMBPressed()) {
        const auto [x, y] = app.GetCursorPosition();
        app.renderer()->canvas()->RegisterPoint(static_cast<float>(x),
                                                static_cast<float>(y));
      }
      app.renderer()->Render();
      app.ui()->Render();
      app.EndFrame();
    }
  } catch (std::exception& e) {
    std::print("{}\n", e.what());
  }

  return 0;
}
