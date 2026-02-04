#ifndef RC_SURFACE_H_
#define RC_SURFACE_H_

#include "aliasing.h"

namespace rc {

class Surface {
  public:
    static Surface& Instnace() {
      static Surface instnace;
      return instnace;
    }

    void Draw();

    ~Surface();

    Surface(const Surface&) = delete;
    void operator=(const Surface&) = delete;

    Surface(Surface&&) = delete;
    void operator=(Surface&&) = delete;

  private:
    Surface();
    u32 vao_;
    u32 vbo_;
};

} // namespace rc

#endif // !RC_SURFACE_H_
