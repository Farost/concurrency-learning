#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity) 
    : sem_(capacity)
    , data_()
  {}

  void Put(T value) 
  {
    const auto token = sem_.Acquire();
    data_.push_back(value, token);
  }

  T Take() 
  {
    const auto& [val, token] = data_.front();
    sem_.Release(token);
    data_.pop_front();
    return val;
  }

 private:
  // Tags
  struct SomeTag {};

 private:
  // Buffer
  TaggedSemaphore<SomeTag> sem_;
  std::deque<std::pair<T, typename TaggedSemaphore<SomeTag>::Token>> data_;
};
