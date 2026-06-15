#ifndef RC_UTILITY_H_
#define RC_UTILITY_H_

#include <memory>

#include "aliasing.h"
#include "glm/fwd.hpp"
#include "texture.h"

namespace rc {

glm::vec4 RandomVec4();

glm::vec3 RandomVec3(float min, float max);

glm::vec2 RandomVec2(float min, float max);

float Random(float min, float max);

std::unique_ptr<Texture> GetNoiseTexture(u64 width, u64 height);

std::unique_ptr<Texture> GetPerlinNoiseTexture(u64 width,
                                               u64 height,
                                               float scale = 100.0f);

std::unique_ptr<Texture> GetFBMTexture(u64 width,
                                       u64 height,
                                       int octaves = 6,
                                       float scale = 5.0f);

}  // namespace rc

#endif  // !RC_UTILITY_H_
