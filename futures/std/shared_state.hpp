#pragma once

#include <memory>
#include <cassert>

#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>

namespace stdlike::detail {

template <typename T>
struct SharedState {

  SharedState() = default;

  // Non-copyable
  SharedState(const SharedState&) = delete;
  SharedState& operator=(const SharedState&) = delete;

  // Movable
  SharedState(SharedState&&) = default;
  SharedState& operator=(SharedState&&) = default;

  //twist::ed::stdlike::atomic<int64_t> refCount;
  twist::ed::stdlike::mutex mutex;
  std::variant<std::monostate, T, std::exception_ptr> value;
  twist::ed::stdlike::condition_variable ready;
};

}  // namespace stdlike::detail
