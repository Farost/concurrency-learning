#pragma once

#include "tagged_semaphore.hpp"

#include <deque>
#include <optional>
#include <iostream>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity)
      : capacity_(capacity)
      , buffer_()
      , emptySlots_(capacity)
      , filledSlots_(0)
      , bufferMutex_(1)
  {
  }

  void Put(T value)
  {
    DataToken emptySlotToken = emptySlots_.Acquire();
    MutexToken mutexToken = bufferMutex_.Acquire();

    buffer_.emplace_back(std::move(value));

    filledSlots_.Release(std::move(emptySlotToken));
    bufferMutex_.Release(std::move(mutexToken));
  }

  T Take()
  {
    DataToken filledSlotToken = filledSlots_.Acquire();
    MutexToken mutexToken = bufferMutex_.Acquire();

    assert(!buffer_.empty());

    T data = std::move(buffer_.front());
    buffer_.pop_front();

    emptySlots_.Release(std::move(filledSlotToken));
    bufferMutex_.Release(std::move(mutexToken));

    return std::move(data);
  }

 private:
  struct DataTag {};
  struct MutexTag {};

  using DataToken = typename TaggedSemaphore<DataTag>::Token;
  using MutexToken = typename TaggedSemaphore<MutexTag>::Token; 

 private:
  size_t capacity_{0};
  std::deque<T> buffer_;

  TaggedSemaphore<DataTag> emptySlots_;
  TaggedSemaphore<DataTag> filledSlots_;

  TaggedSemaphore<MutexTag> bufferMutex_;
};
