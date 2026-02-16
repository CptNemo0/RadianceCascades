#ifndef RC_TEXTURE_H_
#define RC_TEXTURE_H_

#include "glad/include/glad/glad.h"

#include "aliasing.h"

namespace rc {

class Texture {
  public:
    Texture(u64 width, u64 height, void* data = nullptr, i32 bits = GL_RGBA32F,
            i32 format = GL_RGBA, i32 type = GL_FLOAT);
    Texture(const Texture&) = delete;
    void operator=(const Texture&) = delete;
    Texture(Texture&& other) = delete;
    void operator=(Texture&& other) = delete;
    ~Texture();

    u64 id() const {
      return id_;
    }

    u64 width() const {
      return width_;
    }

    u64 height() const {
      return height_;
    }

    void UpdateTexture(void* data);

    // For texture_slot use one of the GL_TEXTURE* macros.
    void BindTexture(int texture_slot) const;

  private:
    u32 id_;
    u64 width_;
    u64 height_;
};

} // namespace rc

#endif // !RC_TEXTURE_H_
