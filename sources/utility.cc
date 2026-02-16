#include "utility.h"

#include "glad/include/glad/glad.h"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/glm.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numbers>
#include <random>
#include <vector>

#include "aliasing.h"
#include "texture.h"

// Those noises are vide coded, they work this is what matters in their case.
namespace {

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> distribution(-0.25f, 0.25f);

float Fade(float t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}

glm::vec2 RandomGradient(int ix, int iy) {
  const unsigned w = 8 * sizeof(unsigned);
  const unsigned s = w / 2;
  unsigned a = ix, b = iy;
  a *= 3284157443;
  b ^= a << s | a >> (w - s);
  b *= 1911520717;
  a ^= b << s | b >> (w - s);
  a *= 2048419325;

  float random = a * (std::numbers::pi_v<float> * 2.0f / ~(~0u >> 1));
  return glm::vec2(std::cos(random), std::sin(random));
}

float DotGridGradient(int ix, int iy, float x, float y) {
  const glm::vec2 gradient = RandomGradient(ix, iy);
  const float dx = x - static_cast<float>(ix);
  const float dy = y - static_cast<float>(iy);
  return (dx * gradient.x + dy * gradient.y);
}

float ComputePerlin(float x, float y) {
  const int x0 = static_cast<int>(std::floor(x));
  const int x1 = x0 + 1;
  const int y0 = static_cast<int>(std::floor(y));
  const int y1 = y0 + 1;

  const float sx = x - static_cast<float>(x0);
  const float sy = y - static_cast<float>(y0);

  return std::lerp(std::lerp(DotGridGradient(x0, y0, x, y),
                             DotGridGradient(x1, y0, x, y), Fade(sx)),
                   std::lerp(DotGridGradient(x0, y1, x, y),
                             DotGridGradient(x1, y1, x, y), Fade(sx)),
                   Fade(sy));
}

float ComputeFBM(float x, float y, int octaves, float lacunarity,
                 float persistence) {
  float total = 0.0f;
  float frequency = 1.0f;
  float amplitude = 1.0f;
  float max_value = 0.0f;

  for (int i = 0; i < octaves; i++) {
    total += ComputePerlin(x * frequency, y * frequency) * amplitude;
    max_value += amplitude;
    amplitude *= persistence;
    frequency *= lacunarity;
  }

  return total / max_value;
}

} // namespace

namespace rc {

std::unique_ptr<Texture> GetNoiseTexture(u64 width, u64 height) {
  std::vector<glm::vec2> data(width * height, glm::vec2(0.0f, 0.0f));
  std::ranges::generate(data, RandomVec4);
  return std::make_unique<Texture>(width, height,
                                   reinterpret_cast<void*>(data.data()),
                                   GL_RG16F, GL_RG, GL_FLOAT);
}

glm::vec4 RandomVec4() {
  return glm::vec4{distribution(gen), distribution(gen), distribution(gen),
                   1.0f};
}

glm::vec2 RandomVec2() {
  return glm::normalize(glm::vec2(distribution(gen), distribution(gen)));
}

std::unique_ptr<Texture> GetPerlinNoiseTexture(u64 width, u64 height,
                                               float scale) {
  std::vector<glm::vec4> data(width * height);

  const float width_scale = scale / width;
  const float height_scale = scale / width;

  for (u64 y = 0; y < height; ++y) {
    for (u64 x = 0; x < width; ++x) {
      const float color_value =
        std::clamp((ComputePerlin(static_cast<float>(x) * width_scale,
                                  static_cast<float>(y) * height_scale) +
                    1.0f) *
                     0.5f,
                   0.0f, 1.0f);

      data[y * width + x] =
        glm::vec4(color_value, color_value, color_value, 1.0f);
    }
  }

  return std::make_unique<Texture>(width, height,
                                   reinterpret_cast<void*>(data.data()));
}

std::unique_ptr<Texture> GetFBMTexture(u64 width, u64 height, int octaves,
                                       float scale) {
  std::vector<float> data(width * height);

  const float lacunarity = 2.0f;
  const float persistence = 0.5f;

  for (u64 y = 0; y < height; ++y) {
    for (u64 x = 0; x < width; ++x) {
      float nx = (static_cast<float>(x) / width) * scale;
      float ny = (static_cast<float>(y) / height) * scale;
      float noise_value = ComputeFBM(nx, ny, octaves, lacunarity, persistence);
      float color_value = (noise_value + 1.0f) * 0.5f;
      color_value = std::clamp(color_value, 0.0f, 1.0f);

      data[y * width + x] = color_value;
    }
  }

  return std::make_unique<Texture>(width, height,
                                   reinterpret_cast<void*>(data.data()),
                                   GL_R16F, GL_RED, GL_FLOAT);
}

} // namespace rc
