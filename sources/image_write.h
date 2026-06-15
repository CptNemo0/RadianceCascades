#ifndef RC_IMAGE_WRITE_H_
#define RC_IMAGE_WRITE_H_

#include "glad/include/glad/glad.h"

#include <format>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "aliasing.h"
#include "stb_image_write.h"

namespace rc {

inline static void SaveFramebufferToPng(std::string_view filepath,
                                        i32 width,
                                        i32 height) {
  const i32 num_channels = 4;
  std::vector<u8> pixels(width * height * num_channels);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  stbi_flip_vertically_on_write(true);

  if (stbi_write_png(filepath.data(), width, height, num_channels,
                     pixels.data(), width * num_channels) == 0) {
    throw std::runtime_error(
        std::format("Error: Failed to write PNG image to {}", filepath));
  }
}

}  // namespace rc

#endif  // !RC_IMAGE_WRITE_H_
