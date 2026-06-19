#ifndef RADIANCE_CASCADES_SAVE_FRAME_JOB_H_
#define RADIANCE_CASCADES_SAVE_FRAME_JOB_H_

#include <array>
#include <utility>

#include "aliasing.h"
#include "constants.h"

namespace rc {

struct SaveFrameJob {
  SaveFrameJob() = default;

  std::array<u8, gScreenHeight * gScreenWidth * 4> pixels{};
  u64 frame_number{0};
  bool valid{false};

  SaveFrameJob(const SaveFrameJob& other) = delete;
  void operator=(const SaveFrameJob& other) = delete;

  SaveFrameJob(SaveFrameJob&& other) noexcept
      : pixels{std::move(other.pixels)} {
    frame_number = std::exchange(other.frame_number, 0);
    valid = std::exchange(other.valid, false);
  }

  void operator=(SaveFrameJob&& other) noexcept {
    pixels = std::move(other.pixels);
    frame_number = other.frame_number;
    valid = other.valid;
  }
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_SAVE_FRAME_JOB_H_
