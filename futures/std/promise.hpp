#pragma once

#include <stdlike/future.hpp>
#include <stdlike/shared_state.hpp>

#include <memory>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise()
    : sharedState_(std::make_shared<detail::SharedState<T>>()) 
  {}

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() 
  {
    return Future<T>(sharedState_);
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T value) 
  {
    std::lock_guard<twist::ed::stdlike::mutex> lock(sharedState_->mutex);

    if (sharedState_->value.index() != 0) 
    {
      throw std::runtime_error("Promise already set");
    }

    sharedState_->value = std::move(value);

    sharedState_->ready.notify_all();
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr exception) 
  {
    std::lock_guard<twist::ed::stdlike::mutex> lock(sharedState_->mutex);

    if (sharedState_->value.index() != 0) 
    {
      throw std::runtime_error("Promise already set");
    }

    sharedState_->value = std::exception_ptr(exception);

    sharedState_->ready.notify_all();
  }

 private:

  std::shared_ptr<detail::SharedState<T>> sharedState_;
};

}  // namespace stdlike
