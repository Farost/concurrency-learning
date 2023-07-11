#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>

#include <exe/coro/core.hpp>
#include "exe/tp/thread_pool.hpp"

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

class Fiber {
 public:
  Fiber(tp::ThreadPool& threadPool, Routine routine);

  void Schedule();

  // Task
  void Run();

  void Suspend();

  static Fiber* Self();

private:
  void Destroy();

 private:
  tp::ThreadPool* threadPool_;
  Routine routine_;
  coro::Coroutine coroutine_;

  static twist::ed::ThreadLocalPtr<Fiber> currentFiber;
};

}  // namespace exe::fibers
