#ifndef RADIANCE_CASCADES_SAVE_FRAME_JOB_H_
#define RADIANCE_CASCADES_SAVE_FRAME_JOB_H_

#include <utility>

#include "aliasing.h"
#include "constants.h"
#include "frame_pool.h"

namespace rc {

struct SaveFrameJob {
  static constexpr u64 frame_size = gScreenHeight * gScreenWidth * 4;

  // Bound to the shared pool even when default-constructed (the ring buffer's
  // nodes start out this way), so that move-assigning a frame in is a pointer
  // swap and the overwritten storage returns to the pool.
  Frame pixels;
  u64 frame_number{0};

  SaveFrameJob() = default;

  SaveFrameJob(Frame frame, u64 number)
      : pixels(std::move(frame)), frame_number(number) {}

  SaveFrameJob(const SaveFrameJob& other) = delete;
  void operator=(const SaveFrameJob& other) = delete;

  SaveFrameJob(SaveFrameJob&& other) noexcept
      : pixels{std::move(other.pixels)},
        frame_number{std::exchange(other.frame_number, 0)} {}

  void operator=(SaveFrameJob&& other) noexcept {
    pixels = std::move(other.pixels);
    frame_number = std::exchange(other.frame_number, 0);
  }
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_SAVE_FRAME_JOB_H_
