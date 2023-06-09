#pragma once

#include <twist/ed/stdlike/mutex.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>

#include <cstdlib>

class Semaphore {
 public:
  explicit Semaphore(size_t tokens) 
    : tokens_(tokens)
    , token_mut_()
    , available_()
  {}

  void Acquire() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(token_mut_);

    while (tokens_ == 0)
    {
      available_.wait(lock);
    }

    tokens_--;
  }

  void Release() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(token_mut_);

    tokens_++;
    
    available_.notify_one();
  }

 private:
  std::size_t tokens_;

  twist::ed::stdlike::mutex token_mut_;
  twist::ed::stdlike::condition_variable available_;
};
