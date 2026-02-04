#ifndef RC_UTILITY_H_
#define RC_UTILITY_H_

#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"

#include <algorithm>
#include <memory>
#include <print>
#include <random>
#include <vector>

#include "aliasing.h"
#include "glm/fwd.hpp"
#include "texture.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> distribution(-0.25f, 0.25f);

namespace rc {

glm::vec4 RandomVec4() {
  return glm::vec4{distribution(gen), distribution(gen), distribution(gen),
                   1.0f};
}

std::unique_ptr<Texture> GetNoiseTexture(u64 width, u64 height) {
  std::vector<glm::vec4> data(width * height,
                              glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  std::print("data.size(): {}\n", data.size());
  std::ranges::generate(data, RandomVec4);
  std::print("Generating done\n");
  return std::make_unique<Texture>(width, height,
                                   reinterpret_cast<void*>(data.data()));
}

} // namespace rc

#endif // !RC_UTILITY_H_
