#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <exception>
#include <print>

#include "app.h"
#include "renderer.h"
#include "ui.h"

int main() {
  try {
    rc::App& app = rc::App::Instance();
    app.Start();
    while (app.ShouldRun()) {
      app.ProcessInput();
      app.scene_renderer()->Render();
      app.ui()->Render();
      app.EndFrame();
    }
  } catch (std::exception& e) {
    std::print("{}\n", e.what());
  }

  return 0;
}
