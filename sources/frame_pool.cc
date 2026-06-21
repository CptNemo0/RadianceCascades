#include "frame_pool.h"

#include <algorithm>
#include <mutex>
#include <optional>
#include <utility>

#include "aliasing.h"

namespace rc {

void FramePool::Init(u64 capacity, u64 frame_size) {
  frames_.resize(capacity);
  std::generate(frames_.begin(), frames_.end(), [&]() {
    Node node;
    node.valid = true;
    node.frame.resize(frame_size);
    return node;
  });
}

std::optional<Frame> FramePool::GetFrame() {
  std::lock_guard lock{mutex_};
  auto valid = std::ranges::find_if(
      frames_, [](FramePool::Node& node) { return node.valid; });
  if (valid == frames_.end()) {
    return std::nullopt;
  }
  valid->valid = false;
  return std::move(valid->frame);
}

void FramePool::ReturnFrame(Frame frame) {
  std::lock_guard lock{mutex_};
  auto valid = std::ranges::find_if(
      frames_, [](FramePool::Node& node) { return !node.valid; });
  if (valid == frames_.end()) {
    return;
  }
  valid->valid = true;
  valid->frame = std::move(frame);
}

}  // namespace rc
