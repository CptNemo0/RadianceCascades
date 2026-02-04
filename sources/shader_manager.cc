#include "shader_manager.h"

#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>

#include "aliasing.h"
#include "constants.h"
#include "shader.h"

namespace {

constexpr std::string_view vertex_extension = ".vs";
constexpr std::string_view fragment_extension = ".fs";

} // namespace

namespace rc {

void ShaderManager::LoadShaders() {
  std::filesystem::path shaders_list = rc::gPathToShadersList;
  if (!std::filesystem::exists(shaders_list)) {
    throw std::runtime_error("Shader list file does not exist.");
    return;
  }

  std::ifstream file(shaders_list);
  if (!file.is_open()) {
    throw std::runtime_error("Couldn't open the shader list file.");
  }
  std::string line;
  std::string vertex_shader;
  std::string fragment_shader;
  while (std::getline(file, line)) {
    ShaderType shader_type;

    try {
      shader_type = ParseShaderType(line);
    } catch (std::exception& e) {
      throw e;
    }

    vertex_shader = std::format("{}{}", line, vertex_extension);
    fragment_shader = std::format("{}{}", line, fragment_extension);
    std::filesystem::path vertex_path =
      shaders_list.parent_path() / vertex_shader;
    std::filesystem::path fragment_path =
      shaders_list.parent_path() / fragment_shader;
    if (!std::filesystem::exists(vertex_path) ||
        !std::filesystem::exists(fragment_path)) {
      throw std::runtime_error(
        std::format("Vertex or fragment {} shader does not exist", line));
    }

    shaders_[static_cast<u64>(shader_type)] =
      std::make_unique<Shader>(vertex_path, fragment_path);
  }
}

const Shader* ShaderManager::GetShader(ShaderType type) const {
  if (type >= ShaderManager::ShaderType::kTypeNum) {
    throw std::runtime_error("GetShader; Passed shader type does not exist");
  }

  return shaders_[static_cast<u64>(type)].get();
}

const Shader* ShaderManager::Use(ShaderManager::ShaderType type) const {
  if (type >= ShaderManager::ShaderType::kTypeNum) {
    throw std::runtime_error("Use: Passed shader type does not exist");
  }

  const Shader* return_value = shaders_[static_cast<u64>(type)].get();
  return_value->use();
  return return_value;
}

} // namespace rc
