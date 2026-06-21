#ifndef RC_ASYNC_IMAGE_WRITER_H_
#define RC_ASYNC_IMAGE_WRITER_H_

#include <format>
#include <memory>
#include <optional>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

#include "aliasing.h"
#include "constants.h"
#include "frame_pool.h"
#include "image_write.h"
#include "pool_ring_buffer.h"
#include "save_frame_job.h"

namespace rc {

class AsyncImageWriter {
 public:
  static constexpr u64 kCapacity = 192;

  explicit AsyncImageWriter(u64 worker_num)
      : jobs_{std::make_unique<PoolRingBuffer<SaveFrameJob>>(kCapacity)} {
    // Pre-create the blocks the steady state needs when the workers keep up
    // (one per worker, one in flight on the render thread, one parked): the
    // push path then allocates nothing. If the queue ever backs up deeper,
    // those extra blocks are allocated once and recycled thereafter (up to
    // kCapacity).
    FramePool::Instance().Init(kCapacity, 4 * gScreenHeight * gScreenWidth);

    worker_threads_.reserve(worker_num);
    for (auto i{0uz}; i < worker_num; ++i) {
      worker_threads_.emplace_back(&AsyncImageWriter::WorkerLoop, this,
                                   stop_source_.get_token());
    }
  }

  AsyncImageWriter(const AsyncImageWriter&) = delete;
  AsyncImageWriter& operator=(const AsyncImageWriter&) = delete;

  // `pixels` is moved all the way into a ring-buffer node: no frame-sized copy
  // and (after warm-up) no allocation on this, the render thread's hot path.
  void PushWork(std::optional<Frame> pixels, u64 frame_number) {
    if (pixels) {
      jobs_->Push(SaveFrameJob{std::move(pixels.value()), frame_number},
                  stop_source_.get_token());
    }
  };

  void StopThreads() { stop_source_.request_stop(); }

  ~AsyncImageWriter() { StopThreads(); }

 private:
  void WorkerLoop(std::stop_token stop) {
    while (!stop.stop_requested()) {
      auto node = jobs_->Pop(stop);
      if (node) {
        WriteRgbaToPng(std::format("output_frames\\{}.png", node->frame_number),
                       node->pixels, gScreenWidth, gScreenHeight);
        FramePool::Instance().ReturnFrame(std::move(node->pixels));
      }
    }
  }

  std::unique_ptr<PoolRingBuffer<SaveFrameJob>> jobs_;
  std::stop_source stop_source_;
  std::vector<std::jthread> worker_threads_;
};

}  // namespace rc

#endif  // !RC_ASYNC_IMAGE_WRITER_H_
