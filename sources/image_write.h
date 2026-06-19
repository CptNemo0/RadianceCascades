#ifndef RC_IMAGE_WRITE_H_
#define RC_IMAGE_WRITE_H_

#include "glad/include/glad/glad.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <span>
#include <stdexcept>
#include <string_view>

#include "aliasing.h"
#include "constants.h"
#include "stb_image_write.h"

namespace rc {

inline void FlipVerticallyInPlace(std::span<u8> pixels,
                                  i32 width,
                                  i32 height,
                                  i32 channels = 4) {
  const std::size_t row_bytes{static_cast<std::size_t>(width) *
                              static_cast<std::size_t>(channels)};
  for (i32 row{0}; row < height / 2; ++row) {
    const std::size_t top{static_cast<std::size_t>(row) * row_bytes};
    const std::size_t bottom{static_cast<std::size_t>(height - 1 - row) *
                             row_bytes};
    std::swap_ranges(pixels.begin() + top, pixels.begin() + top + row_bytes,
                     pixels.begin() + bottom);
  }
}

inline void ReadFramebufferRgba(
    std::array<u8, gScreenHeight * gScreenWidth * 4>& pixels) {
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, gScreenWidth, gScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE,
               pixels.data());
}

inline void WriteRgbaToPng(std::string_view filepath,
                           std::span<u8> pixels,
                           i32 width,
                           i32 height) {
  FlipVerticallyInPlace(pixels, width, height, 4);
  if (stbi_write_png(filepath.data(), width, height, 4, pixels.data(),
                     width * 4) == 0) {
    throw std::runtime_error(
        std::format("Error: Failed to write PNG image to {}", filepath));
  }
}

}  // namespace rc

#endif  // !RC_IMAGE_WRITE_H_
