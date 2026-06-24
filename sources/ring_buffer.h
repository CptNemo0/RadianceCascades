#ifndef RADIANCE_CASCADES_H_
#define RADIANCE_CASCADES_H_

#include <array>
#include <condition_variable>
#include <mutex>
#include <stop_token>

#include "aliasing.h"

namespace rc {

template <typename T, u64 capacity>
class RingBuffer {
 public:
  template <typename Callable>
  void Push(Callable& push_function) {
    static_assert(
        requires(Callable pusher, T& piece_of_data) { pusher(piece_of_data); });
    {
      std::unique_lock lock{mutex_};
      not_full_.wait(lock, [this] { return size_ < capacity; });
      push_function(data_[push_pointer_ % capacity]);
      ++size_;
      ++push_pointer_;
      try_normalize();
    }
    not_empty_.notify_one();
  };

  template <typename Callable>
  void Pop(Callable& pop_function) {
    static_assert(requires(Callable popper, T& piece_of_data, bool is_empty) {
      popper(piece_of_data, is_empty);
    });
    bool popped{false};
    {
      std::lock_guard lock{mutex_};
      pop_function(data_[pop_pointer_ % capacity], empty());
      if (!empty()) {
        --size_;
        ++pop_pointer_;
        try_normalize();
        popped = true;
      }
    }
    if (popped) {
      not_full_.notify_one();
    }
  };

  template <typename Callable>
  bool WaitPop(Callable& pop_function, std::stop_token stop) {
    static_assert(requires(Callable& popper, T& piece_of_data, bool is_empty) {
      popper(piece_of_data, is_empty);
    });
    {
      std::unique_lock lock{mutex_};
      if (!not_empty_.wait(lock, stop, [this] { return !empty(); })) {
        return false;
      }
      pop_function(data_[pop_pointer_ % capacity], empty());
      --size_;
      ++pop_pointer_;
      try_normalize();
    }
    not_full_.notify_one();
    return true;
  };

 private:
  u64 size() const { return size_; }

  bool empty() const { return size() == 0; }

  void try_normalize() {
    if (push_pointer_ > capacity && pop_pointer_ > capacity) {
      push_pointer_ -= capacity;
      pop_pointer_ -= capacity;
    }
  }

  std::mutex mutex_;
  std::condition_variable_any not_empty_;
  std::condition_variable not_full_;

  std::array<T, capacity> data_;
  u64 push_pointer_{};
  u64 pop_pointer_{};
  u64 size_{};
};

}  // namespace rc

#endif  // RADIANCE_CASCADES_H_
