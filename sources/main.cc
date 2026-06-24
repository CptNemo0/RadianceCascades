#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <stop_token>
#include <string>

#include "aliasing.h"
#endif  // !GLFW_INCLUDE_NONE

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif  // !STB_IMAGE_WRITE_IMPLEMENTATION

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif  // !STB_IMAGE_WRITE_IMPLEMENTATION

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdouble-promotion"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif
#include "stb_image.h"
#include "stb_image_write.h"
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include "glad/include/glad/glad.h"

#include <GLFW/glfw3.h>

#include <exception>
#include <print>

#include "app.h"
#include "pool_ring_buffer.h"
#include "renderer.h"
#include "ui.h"

int main() {
  try {
    rc::App& app{rc::App::Instance()};
    app.Start();
    while (app.ShouldRun()) {
      app.StartFrame();
      app.scene_renderer()->Render();
      app.ui()->Render();
      app.EndFrame();
    }
  } catch (std::exception& error) {
    std::println("{}", error.what());
  }

  return 0;
}
