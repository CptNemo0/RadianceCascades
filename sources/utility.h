#ifndef RC_UTILITY_H_
#define RC_UTILITY_H_

#include "glm/fwd.hpp"

#include <memory>

#include "aliasing.h"
#include "texture.h"

namespace rc {

glm::vec4 RandomVec4();

std::unique_ptr<Texture> GetNoiseTexture(u64 width, u64 height);

} // namespace rc

#endif // !RC_UTILITY_H_
