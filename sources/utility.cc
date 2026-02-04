#include "utility.h"

#include "glm/glm.hpp"

#include <algorithm>
#include <memory>
#include <random>
#include <vector>

#include "aliasing.h"
#include "glm/fwd.hpp"
#include "texture.h"

namespace {

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> distribution(-0.25f, 0.25f);

} // namespace

namespace rc {

std::unique_ptr<Texture> GetNoiseTexture(u64 width, u64 height) {
  std::vector<glm::vec4> data(width * height,
                              glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
  std::ranges::generate(data, RandomVec4);
  return std::make_unique<Texture>(width, height,
                                   reinterpret_cast<void*>(data.data()));
}

glm::vec4 RandomVec4() {
  return glm::vec4{distribution(gen), distribution(gen), distribution(gen),
                   1.0f};
}

} // namespace rc
