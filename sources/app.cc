#include "app.h"

#include "glad/include/glad/glad.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <memory>
#include <stdexcept>

#include "aliasing.h"
#include "constants.h"
#include "glm/ext/vector_float2.hpp"
#include "measurement_manager.h"
#include "renderer.h"
#include "shader_manager.h"
#include "timed_scope.h"
#include "ui.h"

namespace rc {

App::App() {}

App::~App() {
  glfwTerminate();
}

void App::Start() {
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

  glfwSwapInterval(0);

  StartImpl();
}

void App::StartFrame() {
  if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window_, true);
  }

  if (RMBPressed()) {
    RegisterMousePosition();
  }

  if (IsMeasuring()) {
    if (IsMeasuring()) {
      time_ = static_cast<float>(frames_measured_) * 0.01f;
    }
  } else {
    time_ = static_cast<float>(glfwGetTime()) * 1000.f - time_normalizer_;
    if (time_ > 1000.0f) {
      time_normalizer_ += 1000.0f;
    }
    time_ *= 0.001f;
  }
}

bool App::ShouldRun() {
  return !glfwWindowShouldClose(window_);
};

glm::vec2 App::GetCursorPosition() {
  double position_x{};
  double position_y{};
  glfwGetCursorPos(window_, &position_x, &position_y);
  return {static_cast<float>(position_x), static_cast<float>(position_y)};
}

bool App::RMBPressed() {
  return glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

void App::EndFrame() {
  TimedScope scope{nullptr};
  glfwSwapBuffers(window_);
  glfwPollEvents();
  if (!is_measuring_) {
    return;
  }
  if (frames_measured_ == gFramesToMeasure - 1) {
    StopMeasuring();
    return;
  }
  frames_measured_++;
}

void App::AddObserver(App::Observer* observer) {
  observers_.push_back(observer);
}

void App::RemoveObserver(App::Observer* observer) {
  if (const auto search{
          std::find(observers_.begin(), observers_.end(), observer)};
      search != observers_.end()) {
    observers_.erase(search);
  }
}

void App::RegisterMousePosition() {
  const glm::vec2 position{GetCursorPosition()};
  for (auto* observer : observers_) {
    observer->GetMousePositionOnRMB(position);
  }
}

void App::StartMeasuring() {
  frames_measured_ = 0;
  is_measuring_ = true;
  measurement_manager_->StartMeasuring(gFramesToMeasure);
}

void App::StopMeasuring() {
  measurement_manager_->StopMeasuring();
  is_measuring_ = false;
}

bool App::IsMeasuring() const {
  return is_measuring_;
}

u64 App::MeasuredFrameIndex() const {
  return frames_measured_;
}

void App::StartImpl() {
  ShaderManager::Instance().LoadShaders();
  renderer_ = std::make_unique<Renderer>();
  measurement_manager_ = std::make_unique<MeasurementManager>();
  renderer_->Initialize();
  ui_ = std::make_unique<Ui>(window_, renderer_.get());
}

}  // namespace rc
