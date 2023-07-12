#pragma once

#include <exe/fibers/core/routine.hpp>
#include <exe/fibers/core/scheduler.hpp>
#include <twist/ed/local/ptr.hpp>

#include <exe/coro/core.hpp>

#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/strand.hpp>
#include <twist/ed/stdlike/condition_variable.hpp>
#include <twist/ed/stdlike/mutex.hpp>
#include "asio/steady_timer.hpp"

namespace exe::fibers {

// Fiber = stackful coroutine + scheduler (thread pool)

using Millis = std::chrono::milliseconds;

class Fiber {
 public:
  Fiber(Scheduler& scheduler, Routine routine);

  void Schedule();

  // Task
  void Run();

  void Suspend();

  void SleepFor(Millis delay);

  static Fiber* Self();

  static Scheduler* GetSelfScheduler();

private:
  void Destroy();

 private:
  Scheduler* scheduler_;
  Routine routine_;
  coro::Coroutine coroutine_;
  asio::strand<asio::io_context::executor_type> strand_;

  static twist::ed::ThreadLocalPtr<Fiber> currentFiber;
};

}  // namespace exe::fibers
