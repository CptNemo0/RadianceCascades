#ifndef RC_RENDER_TARGET_H_
#define RC_RENDER_TARGET_H_

#include "glad/include/glad/glad.h"

#include "aliasing.h"
#include "texture.h"

namespace rc {

class RenderTarget {
  public:
    RenderTarget(u64 width, u64 height, i32 bits = GL_RGBA32F,
                 i32 format = GL_RGBA, i32 type = GL_FLOAT);
    RenderTarget(const RenderTarget&) = delete;
    void operator=(const RenderTarget&) = delete;
    RenderTarget(RenderTarget&&) = delete;
    void operator=(RenderTarget&&) = delete;
    ~RenderTarget();

    void Bind() const;

    void Clear(int flags = GL_COLOR_BUFFER_BIT) const;

    void BindTexture(int texture_slot) const;

    static void BindDefault() {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static void ClearDefault(int flags = GL_COLOR_BUFFER_BIT) {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(flags);
    }

  private:
    Texture texture_;
    u32 frame_buffer_handle_;
    u32 render_buffer_handle_;
};

} // namespace rc

#endif // !RC_RENDER_TARGET_H_
