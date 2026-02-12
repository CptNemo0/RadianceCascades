#ifndef RC_UI_H_
#define RC_UI_H_

struct GLFWwindow;

namespace rc {

class Renderer;
class FlameGenerator;

class Ui {
  public:
    explicit Ui(GLFWwindow* window, Renderer* renderer);
    ~Ui();
    void Render();

  private:
    Renderer* renderer_;
    FlameGenerator* flame_generator_;
};

} // namespace rc

#endif // !RC_UI_H_
