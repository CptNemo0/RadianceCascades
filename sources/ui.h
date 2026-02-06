#ifndef RC_UI_H_
#define RC_UI_H_

#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"

#include "aliasing.h"

struct GLFWwindow;

namespace rc {

class Renderer;

class Ui {
  public:
    explicit Ui(GLFWwindow* window, Renderer* renderer);
    ~Ui();
    void Render();

  private:
    GLFWwindow* window_ = nullptr;
    Renderer* renderer_;

    glm::vec3 brush_color_;
    float brush_size_;
};

} // namespace rc

#endif // !RC_UI_H_
