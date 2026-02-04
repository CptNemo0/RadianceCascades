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

inline constexpr u32 gScreenWidth = 256 * 3;
inline constexpr u32 gScreenHeight = 256 * 3;

inline constexpr float gOneOverWidth = 1.0f / gScreenWidth;
inline constexpr float gOneOverHeight = 1.0f / gScreenHeight;

// Find first power of 2 that's greater than the greater of two dimensions.
// gSteps in JFA paper is counterpart of variable N in the article.
// std::log2 will be constexpr in C++26. Imagine not making it constexpr out of
// the box. Typical C++ L.
inline const u64 gJfaSteps =
  std::ceil(std::log2(std::max(gScreenHeight, gScreenWidth)));
inline const u64 gCascadeLevels = 7;
inline const u64 gBaseRayCount = 4;

inline constexpr u64 gMaxBrushRadius = 50;

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
