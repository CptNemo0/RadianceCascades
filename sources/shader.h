#ifndef SHADER_H
#define SHADER_H

#include "glad/include/glad/glad.h"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "aliasing.h"

namespace rc {

// This is ai-generated. I don't care to write this class for the 100th time.
class Shader {
  public:
    // The Program ID
    unsigned int id_;
    std::unordered_map<std::string, i32> locations_;

    void ParseForUniforms(const std::string& code) {
      // Regex breakdown:
      // \buniform  : Matches the word "uniform" at a word boundary
      // \s+        : Matches one or more whitespace characters
      // [a-zA-Z_][\w]* : Matches the type (e.g., mat4, vec3, sampler2D)
      // \s+        : Matches one or more whitespace characters
      // ([a-zA-Z_][\w]*) : Captures the variable name (Group 1)
      std::regex uniformRegex(
        R"(\buniform\s+[a-zA-Z_][\w]*\s+([a-zA-Z_][\w]*))");

      auto words_begin =
        std::sregex_iterator(code.begin(), code.end(), uniformRegex);
      auto words_end = std::sregex_iterator();

      for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string name = match[1].str();

        // Insert into the map with 0 as the initial value
        locations_[name] = 0;
      }
    }

    // Constructor reads and builds the shader
    Shader(const std::filesystem::path& vertexPath,
           const std::filesystem::path& fragmentPath) {
      // 1. Retrieve the vertex/fragment source code from filePath
      std::string vertexCode;
      std::string fragmentCode;
      std::print("{} {}\n", vertexPath.string(), fragmentPath.string());
      try {
        vertexCode = readFile(vertexPath);
        fragmentCode = readFile(fragmentPath);
      } catch (const std::exception& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what()
                  << std::endl;
        return;
      }

      ParseForUniforms(vertexCode);
      ParseForUniforms(fragmentCode);

      const char* vShaderCode = vertexCode.c_str();
      const char* fShaderCode = fragmentCode.c_str();

      // 2. Compile shaders
      unsigned int vertex, fragment;

      // Vertex Shader
      vertex = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex, 1, &vShaderCode, nullptr);
      glCompileShader(vertex);
      checkCompileErrors(vertex, "VERTEX");

      // Fragment Shader
      fragment = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment, 1, &fShaderCode, nullptr);
      glCompileShader(fragment);
      checkCompileErrors(fragment, "FRAGMENT");

      // 3. Shader Program
      id_ = glCreateProgram();
      glAttachShader(id_, vertex);
      glAttachShader(id_, fragment);
      glLinkProgram(id_);
      checkCompileErrors(id_, "PROGRAM");

      // 4. Delete the shaders as they're linked into our program now and no
      // longer necessary
      glDeleteShader(vertex);
      glDeleteShader(fragment);

      for (auto& [name, value] : locations_) {
        value = glGetUniformLocation(id_, name.c_str());
      }
    }

    // Activate the shader
    void use() const {
      glUseProgram(id_);
    }

    // Utility uniform functions
    void setBool(const std::string& name, bool value) const {
      glUniform1i(locations_.at(name), static_cast<int>(value));
    }

    void setInt(const std::string& name, int value) const {
      glUniform1i(locations_.at(name), value);
    }

    void setFloat(const std::string& name, float value) const {
      glUniform1f(locations_.at(name), value);
    }

    void setVec2(const std::string& name, const glm::vec2& value) const {
      glUniform2f(locations_.at(name), value.x, value.y);
    }

    void setVec3(const std::string& name, const glm::vec3& value) const {
      glUniform3fv(locations_.at(name), 1, glm::value_ptr(value));
    }

  private:
    // Helper to read file content into string
    std::string readFile(const std::filesystem::path& path) {
      std::ifstream file;
      // Ensure ifstream objects can throw exceptions:
      file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

      try {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
      } catch (std::ifstream::failure& e) {
        throw std::runtime_error("File not read successfully: " +
                                 path.string());
      }
    }

    // Helper to check for compilation/linking errors
    void checkCompileErrors(unsigned int shader, std::string type) {
      int success;
      char infoLog[1024];
      if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
          glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
          std::cerr
            << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- -- "
            << std::endl;
        }
      } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
          glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
          std::cerr
            << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
            << infoLog
            << "\n -- --------------------------------------------------- -- "
            << std::endl;
        }
      }
    }
};

} // namespace rc

#endif
