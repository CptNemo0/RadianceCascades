#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif // !STB_IMAGE_WRITE_IMPLEMENTATION

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#include "stb_image_write.h"
#pragma clang diagnostic pop

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
      app.StartFrame();
      app.scene_renderer()->Render();
      app.ui()->Render();
      app.EndFrame();
    }
  } catch (std::exception& e) {
    std::print("{}\n", e.what());
  }

  return 0;
}
