#ifndef RC_CONSTANTS_H_
#define RC_CONSTANTS_H_

#include "aliasing.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <string_view>

namespace rc {
inline constexpr std::string_view gWindowName = "Radiance Cascades";
inline constexpr std::string_view gPathToShadersList = "shaders\\shaders.list";

inline constexpr u32 gScreenWidth = 256 * 4;
inline constexpr u32 gScreenHeight = 256 * 4;

inline constexpr float gOneOverWidth = 1.0f / gScreenWidth;
inline constexpr float gOneOverHeight = 1.0f / gScreenHeight;

// Find first power of 2 that's greater than the greater of two dimensions.
// gSteps in JFA paper is counterpart of variable N in the article.
// std::log2 will be constexpr in C++26. Imagine not making it constexpr out of
// the box. Typical C++ L.
inline const u64 gJfaSteps =
  std::ceil(std::log2(std::max(gScreenHeight, gScreenWidth)));

inline constexpr u64 gMaxBrushRadius = 50;

inline const f32 gBrushScale =
  1.0f /
  std::sqrtf((gScreenWidth * gScreenWidth) + (gScreenHeight * gScreenHeight));

// 10% of the screen
inline constexpr f32 gMaxFlameSize = 0.6;

inline constexpr u64 gMaxFlameCount = 16;

inline constexpr u64 gFramesToMeasure = 5 * 1024;

// Full-screen quad vertices:
// Positions (x, y, z) | Texture Coords (u, v)
inline constexpr std::array<f32, 30> full_screen_quad_vertices = {
  -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,

  -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,

  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,

  -1.0f, 1.0f,  0.0f, 0.0f, 1.0f,

  1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,

  1.0f,  1.0f,  0.0f, 1.0f, 1.0f};

} // namespace rc

#endif // !RC_CONSTANTS_H_
