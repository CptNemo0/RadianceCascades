#include "texture.h"

#include "glad/include/glad/glad.h"

#include <iostream>

#include "aliasing.h"
#include "constants.h"

namespace rc {

Texture::Texture(u64 width, u64 height, void* data)
  : width_(width), height_(height) {
  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
               GL_FLOAT, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);
}

Texture::~Texture() {
  glDeleteTextures(1, &id_);
}

void Texture::UpdateTexture(void* data) {
  if (!data) {
    std::cerr << "Passed in empty data pointer to UpdateTexture\n";
    return;
  }
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rc::gScreenWidth, rc::gScreenHeight,
                  GL_RGB, GL_UNSIGNED_BYTE, data);
}

// For texture_slot use one of the GL_TEXTURE* macros.
void Texture::BindTexture(int texture_slot) const {
  glActiveTexture(texture_slot);
  glBindTexture(GL_TEXTURE_2D, id_);
}

} // namespace rc
