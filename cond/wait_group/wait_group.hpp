#pragma once

#include <cstdlib>
#include <mutex>

#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>

class WaitGroup {
 public:
  // += count
  void Add(size_t count) {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    count_ += count;
  }

  // =- 1
  void Done() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    count_--;

    if (count_ <= 0)
    {
      zeroCount_.notify_all();
    }
  }

  // == 0
  // One-shot
  void Wait() {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

    zeroCount_.wait(
      lock, 
      [this]()
      {
        return count_ == 0;
      });
  }

 private:
  int64_t count_{0};
  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable zeroCount_;
};
