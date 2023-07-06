#pragma once

#include <memory>
#include <cassert>

#include <stdlike/shared_state.hpp>

namespace stdlike {

template <typename T>
class Future {
  template <typename U>
  friend class Promise;

 public:
  // Non-copyable
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  // Movable
  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  // One-shot
  // Wait for result (value or exception)
  T Get() 
  {
    std::unique_lock<twist::ed::stdlike::mutex> lock(sharedState_->mutex);

    sharedState_->ready.wait(
      lock, 
      [this]() 
      {
        return sharedState_->value.index() != 0;
      });

    if (sharedState_->value.index() == 1) 
    {
      return std::move(std::get<1>(sharedState_->value));
    } 

    std::rethrow_exception(std::move(std::get<2>(sharedState_->value)));
  }

 private:
  explicit Future(std::shared_ptr<detail::SharedState<T>> sharedState)
    : sharedState_(std::move(sharedState)) 
  {}

 private:
  std::shared_ptr<detail::SharedState<T>> sharedState_;
};

}  // namespace stdlike
