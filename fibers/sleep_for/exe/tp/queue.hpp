#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <optional>
#include <queue>

namespace exe::tp {

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T task) 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    if (!working_)
    {
      return false;
    }

    data_.push(std::move(task));
    dataReady_.notify_all();

    return true;
  }

  std::optional<T> Take() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    while (data_.empty())
    {
      if (!working_)
      {
        return std::nullopt;
      }

      dataReady_.wait(lock);
    }

    T task = std::move(data_.front());
    data_.pop();

    return task;
  }

  void Close() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    working_ = false;
    dataReady_.notify_all();
  }

 private:
  bool working_{true};
  std::queue<T> data_;

  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable dataReady_;
};

}  // namespace exe::tp
