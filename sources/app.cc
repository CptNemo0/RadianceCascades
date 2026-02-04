#include "app.h"

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include "constants.h"
#include "renderer.h"
#include "shader_manager.h"

namespace rc {

App::App() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window_ = glfwCreateWindow(rc::gScreenWidth, rc::gScreenHeight,
                             rc::gWindowName.data(), nullptr, nullptr);

  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(window_);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    glfwTerminate();
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glfwSwapInterval(1);

  ShaderManager::Instance().LoadShaders();
  renderer_ = std::make_unique<Renderer>();
  renderer_->Initialize();
}

App::~App() {
  glfwTerminate();
}

void App::ProcessInput() {
  if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window_, true);
  }
}

bool App::ShouldRun() {
  return !glfwWindowShouldClose(window_);
};

std::pair<float, float> App::GetCursorPosition() {
  double position_x{};
  double position_y{};
  glfwGetCursorPos(window_, &position_x, &position_y);
  return {static_cast<float>(position_x), static_cast<float>(position_y)};
}

bool App::LMBPressed() {
  return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

void App::EndFrame() {
  glfwSwapBuffers(window_);
  glfwPollEvents();
}

float App::GetTime() {
  return static_cast<float>(glfwGetTime());
}

} // namespace rc
