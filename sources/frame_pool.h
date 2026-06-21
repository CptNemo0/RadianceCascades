#ifndef RADIANCE_CASCADES_FRAME_POOL_H_
#define RADIANCE_CASCADES_FRAME_POOL_H_

#include <mutex>
#include <optional>
#include <vector>

#include "aliasing.h"

namespace rc {

using Frame = std::vector<u8>;

class FramePool {
 public:
  struct Node {
    Frame frame;
    bool valid{false};
  };

  static FramePool& Instance() {
    static FramePool pool;
    return pool;
  }

  void Init(u64 capacity, u64 frame_size);

  std::optional<Frame> GetFrame();

  void ReturnFrame(Frame frame);

  FramePool(const FramePool&) = delete;
  FramePool(FramePool&&) = delete;
  void operator=(const FramePool&) = delete;
  void operator=(FramePool&&) = delete;

  ~FramePool() = default;

 private:
  FramePool() = default;

  std::mutex mutex_;
  std::vector<Node> frames_;
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_FRAME_POOL_H_
