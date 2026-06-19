#ifndef RC_ASYNC_IMAGE_WRITER_H_
#define RC_ASYNC_IMAGE_WRITER_H_

#include <array>
#include <format>
#include <memory>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

#include "aliasing.h"
#include "constants.h"
#include "image_write.h"
#include "ring_buffer.h"
#include "save_frame_job.h"

namespace rc {

// Encodes and writes saved frames to disk on a pool of worker threads so the
// render thread never blocks on PNG compression or file I/O. Frames cross over
// through a bounded RingBuffer. Each worker is driven by its jthread's own
// stop_token, so both an explicit StopThreads() and ordinary destruction wake
// the workers, drain whatever is still queued, and join cleanly.
class AsyncImageWriter {
 public:
  explicit AsyncImageWriter(u64 worker_num)
      : work_{std::make_unique<RingBuffer<SaveFrameJob, 128>>()} {
    worker_threads_.reserve(worker_num);
    for (auto i{0uz}; i < worker_num; ++i) {
      worker_threads_.emplace_back(
          [this](std::stop_token stop) { WorkerLoop(stop); });
    }
  }

  AsyncImageWriter(const AsyncImageWriter&) = delete;
  AsyncImageWriter& operator=(const AsyncImageWriter&) = delete;

  void PushWork(std::array<u8, gScreenHeight * gScreenWidth * 4>&& pixels,
                u64 frame_number) {
    auto pusher_function = [&](SaveFrameJob& job) {
      job.pixels = std::move(pixels);
      job.frame_number = frame_number;
      job.valid = true;
    };
    work_->Push(pusher_function);
  };

  void StopThreads() {
    for (std::jthread& worker : worker_threads_) {
      worker.request_stop();
    }
  }

  ~AsyncImageWriter() { StopThreads(); }

 private:
  void WorkerLoop(std::stop_token stop) {
    std::unique_ptr<SaveFrameJob> job_ptr = std::make_unique<SaveFrameJob>();
    SaveFrameJob& job = *job_ptr;

    auto pop_function = [&](SaveFrameJob& incoming_data, bool /*is_empty*/) {
      job = std::move(incoming_data);
    };

    while (work_->WaitPop(pop_function, stop)) {
      if (!job.valid) {
        continue;
      }
      WriteRgbaToPng(std::format("output_frames\\{}.png", job.frame_number),
                     job.pixels, gScreenWidth, gScreenHeight);
    }
  }

  std::unique_ptr<RingBuffer<SaveFrameJob, 128>> work_;
  std::vector<std::jthread> worker_threads_;
};

}  // namespace rc

#endif  // !RC_ASYNC_IMAGE_WRITER_H_
