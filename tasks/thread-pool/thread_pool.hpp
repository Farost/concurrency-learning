#pragma once

#include <tp/queue.hpp>
#include <tp/task.hpp>

#include <twist/ed/stdlike/thread.hpp>
#include "twist/ed/local/ptr.hpp"

namespace tp {

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // Launches worker threads
  void Start();

  // Schedules task for execution in one of the worker threads
  void Submit(Task);

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

  // Waits until outstanding work count reaches zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  void Stop();

 private:
 void IncrementActiveTasks();
 void DecrementActiveTasks();

 private:
  std::vector<twist::ed::stdlike::thread> workers_;
  UnboundedBlockingQueue<Task> tasks_;
  bool stop_;
  size_t activeTaskCount_;

  twist::ed::stdlike::mutex mutex_;
  twist::ed::stdlike::condition_variable cv_;

  static twist::ed::ThreadLocalPtr<ThreadPool> currentPool_;
};

}  // namespace tp
