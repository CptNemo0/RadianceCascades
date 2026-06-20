#ifndef RADIANCE_CASCADES_BOUNCERS_H_
#define RADIANCE_CASCADES_BOUNCERS_H_

#include "glm/ext/vector_float3.hpp"

class Entity {
 public:
  const glm::vec3& position() const { return position_; }

  void position(const glm::vec3& position) { position_ = position; }

 private:
  glm::vec3 position_;
};

#endif  // ! RADIANCE_CASCADES_BOUNCERS_H_
