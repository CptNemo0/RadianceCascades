#ifndef RC_SHADER_MANAGER_H_
#define RC_SHADER_MANAGER_H_

#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "shader.h"

namespace rc {

class ShaderManager {
  public:
    enum class ShaderType {
      // Copies pixels from a texture to a render target.
      kSurface = 0,
      // Fills the texture in the radius around the cursor with the color of the
      // brush. Ping-pong
      kCanvas = 1,
      // Each drawn pixel gets assigned value corresponding to (uv.x, yv.y, 0.0)
      kUvColorspace = 2,
      // Jump flood algorithm
      kJfa = 3,
      // Signed distance field
      kSdf = 4,
      // Global illumination - standard approach
      kGi = 5,
      // Global illumination - radiance cascades approach
      kRc = 6,
      // Flame shader - produces a singular flame in the middle of the scene.
      kFlame = 7,
      kOverlay = 8,
      kTypeNum
    };

    static ShaderType ParseShaderType(std::string_view value) {
      if (value == "surface") {
        return ShaderType::kSurface;
      }

      if (value == "canvas") {
        return ShaderType::kCanvas;
      }

      if (value == "to_uv_colorspace") {
        return ShaderType::kUvColorspace;
      }

      if (value == "jfa") {
        return ShaderType::kJfa;
      }

      if (value == "sdf") {
        return ShaderType::kSdf;
      }

      if (value == "global_illumination") {
        return ShaderType::kGi;
      }

      if (value == "radiance_cascade") {
        return ShaderType::kRc;
      }

      if (value == "flame") {
        return ShaderType::kFlame;
      }

      if (value == "overlay") {
        return ShaderType::kOverlay;
      }

      throw std::runtime_error(
        std::format("Shader type: {} does not exist", value));
    }

    static ShaderManager& Instance() {
      static ShaderManager instance;
      return instance;
    }

    void LoadShaders();

    const Shader* GetShader(ShaderType type) const;

    const Shader* Use(ShaderType type) const;

  private:
    ShaderManager() = default;

    std::array<std::unique_ptr<rc::Shader>,
               static_cast<std::size_t>(ShaderManager::ShaderType::kTypeNum)>
      shaders_;
};

} // namespace rc

#endif // !RC_SHADER_MANAGER_H_
