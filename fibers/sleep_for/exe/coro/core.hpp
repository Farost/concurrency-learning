#pragma once

#include <sure/context.hpp>
#include <sure/stack.hpp>

#include <function2/function2.hpp>

#include <exception>

namespace exe::coro {

class Coroutine : sure::ITrampoline
{
 public:
  using Routine = fu2::unique_function<void()>;

  explicit Coroutine(Routine routine);

  void Resume();

  // Suspend running coroutine
  static void Suspend();

  bool IsCompleted() const;
 
 private:
  virtual void Run() noexcept override;
  void SuspendInternal(); 

 private:
  static thread_local Coroutine* currentCoroutine;

  sure::ExecutionContext context_;
  sure::ExecutionContext outerContext_;

  Routine routine_;
  sure::Stack stack_;
  std::exception_ptr exception_;
  bool completed_{false};
};

}  // namespace exe::coro
