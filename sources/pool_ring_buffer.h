#ifndef RADIANCE_CASCADES_POOL_RING_BUFFER_H_
#define RADIANCE_CASCADES_POOL_RING_BUFFER_H_

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stop_token>
#include <utility>

#include "aliasing.h"

namespace rc {

template <typename T>
class PoolRingBuffer {
 public:
  struct Node {
    Node* next_{};
    Node* previous_{};
    T value_{};
    bool valid_{false};
    bool borrowed_{true};
  };

  struct BorrowedNode {
    PoolRingBuffer* source_{nullptr};
    Node* node_{nullptr};

    BorrowedNode() = default;

    explicit BorrowedNode(PoolRingBuffer* source, Node* node)
        : source_(source), node_(node) {}

    void Release() {
      if (source_ && node_) {
        source_->ReturnNode(node_);
      }
    }

    BorrowedNode(BorrowedNode&& other) noexcept
        : source_(std::exchange(other.source_, nullptr)),
          node_(std::exchange(other.node_, nullptr)) {}

    void operator=(BorrowedNode&& other) noexcept {
      if (this != &other) {
        Release();
        source_ = std::exchange(other.source_, nullptr);
        node_ = std::exchange(other.node_, nullptr);
      }
    }

    BorrowedNode(const BorrowedNode&) = delete;
    void operator=(const BorrowedNode&) = delete;

    ~BorrowedNode() { Release(); }

    explicit operator bool() const { return node_ && source_; }
    T* operator->() const { return &node_->value_; }
  };

  explicit PoolRingBuffer(u64 capacity)
      : capacity_(std::max(capacity, 3uz)),
        nodes_(new Node[capacity_]),
        push_pointer_(nullptr),
        pop_pointer_(nullptr) {
    ConnectNodes();
  }

  void Push(T new_value, std::stop_token token) {
    {
      std::unique_lock lock{mutex_};
      if (!not_full_.wait(lock, token, [&] { return !full(); })) {
        return;
      }

      push_pointer_->value_ = std::move(new_value);
      push_pointer_->valid_ = true;
      push_pointer_ = push_pointer_->next_;
      ++size_;
    }
    not_empty_.notify_one();
  }

  BorrowedNode Pop(std::stop_token token) {
    std::unique_lock lock{mutex_};
    if (!not_empty_.wait(lock, token, [&] { return !empty(); })) {
      return {};
    }
    Node* previous = pop_pointer_->previous_;
    Node* to_be_popped = pop_pointer_;
    Node* next = pop_pointer_->next_;

    previous->next_ = next;
    next->previous_ = previous;

    // Popping LAST node from the list, should null out pointers - internal
    // pointers should only point to nodes of the list that are not borrowed.
    if (next == to_be_popped) {
      pop_pointer_ = nullptr;
      push_pointer_ = nullptr;
    } else {
      // Push and Pop pointer are pointing at the same thing either when the
      // queue is empty or full ie. pop pointer did a full circle around the
      // queue and has come back to the start - ie. pop pointer.
      //
      // When the queue is full popping doesn't change the fact that it's full
      // - it makes it smaller since the popped node is borrowed. Thus when
      // moving pop pointer forward, push pointer must also be moved forward.
      // Unless push pointer is moved it will point to the borrowed node.
      if (push_pointer_ == pop_pointer_) {
        push_pointer_ = pop_pointer_->next_;
      }
      pop_pointer_ = pop_pointer_->next_;
    }
    --size_;
    ++borrowed_;
    to_be_popped->borrowed_ = true;

    not_full_.notify_one();
    assert(to_be_popped->valid_);
    return BorrowedNode{this, to_be_popped};
  }

  void ReturnNode(Node* node) {
    {
      std::lock_guard lock{mutex_};

      if (!ValidateNodeAddress(node)) {
        return;
      }

      node->borrowed_ = false;
      node->valid_ = false;

      // Push pointer is null only when all of the nodes were borrowed.
      if (push_pointer_ == nullptr) {
        node->next_ = node;
        node->previous_ = node;
        push_pointer_ = node;
        pop_pointer_ = node;
      } else {
        Node* previous = push_pointer_->previous_;
        Node* current = push_pointer_;

        previous->next_ = node;
        node->previous_ = previous;

        node->next_ = current;
        current->previous_ = node;

        push_pointer_ = node;
      }

      // Even if all nodes are returned it might be the case that they are also
      // all invalid ie. they do not hold actual values, just garbage. In such
      // case, size is zero. This is the case from initialization. Thus pop and
      // push pointer should point to the same thing.
      if (size_ == 0) {
        pop_pointer_ = push_pointer_;
      }
      --borrowed_;
    }

    not_full_.notify_one();
  }

 private:
  u64 capacity() const { return capacity_; }

  u64 size() const { return size_; };

  u64 empty() const { return size() == 0; }

  u64 full() const { return size_ == capacity_ - borrowed_; }

  void ConnectNodes() {
    borrowed_ = capacity_;
    for (u64 i{}; i < capacity_; ++i) {
      ReturnNode(&nodes_[i]);
    }
  }

  bool ValidateNodeAddress(const Node* node) const {
    auto cast = [](const Node* to_cast) static -> uintptr_t {
      return reinterpret_cast<uintptr_t>(to_cast);
    };

    const uintptr_t start_address{cast(nodes_.get())};
    const uintptr_t check{cast(node)};

    // Trying to return address from outside the nodes_ block.
    if (start_address > check || cast(&nodes_[capacity_ - 1]) < check) {
      return false;
    }

    // Check alignment, or rather lack of it.
    const uintptr_t offset{check - start_address};
    if (offset % node_size_ != 0) {
      return false;
    }

    // Check if the node was actually borrowed.
    return nodes_[offset / node_size_].borrowed_;
  }

  static constexpr u64 node_size_{sizeof(Node)};

  mutable std::mutex mutex_;
  std::condition_variable_any not_full_;
  std::condition_variable_any not_empty_;

  u64 capacity_;
  u64 size_{};
  u64 borrowed_;
  std::unique_ptr<Node[]> nodes_;
  Node* push_pointer_;
  Node* pop_pointer_;
};

}  // namespace rc

#endif  // ! RADIANCE_CASCADES_POOL_RING_BUFFER_H_
