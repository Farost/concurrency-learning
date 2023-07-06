#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t participants)
    : expParticipants_(participants)
    , curParticipants_(0)
    , epoch_(0) 
  {}

  void ArriveAndWait() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(mut_);

    size_t curEpoch = epoch_;
    ++curParticipants_;

    if (curParticipants_ < expParticipants_) 
    {
      gatheredAll_.wait(lock, [&]() { return epoch_ != curEpoch; });
    } 
    else
    {
      // For Production: make grade cyclic (numeric_limits::max + 1 = 0).
      ++epoch_;
      curParticipants_ = 0;

      gatheredAll_.notify_all();
    }
  }

 private:
  size_t expParticipants_{0};
  size_t curParticipants_{0};
  size_t epoch_{0};

  twist::ed::stdlike::mutex mut_;
  twist::ed::stdlike::condition_variable gatheredAll_;
};