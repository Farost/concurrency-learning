#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

#include <asio/steady_timer.hpp>

#include <iostream>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> Fiber::currentFiber = nullptr;

Fiber::Fiber(Scheduler& scheduler, Routine routine) 
  : scheduler_(&scheduler)
  , routine_(std::move(routine))
  , coroutine_([&](){ routine_(); })
  , strand_(scheduler.get_executor())
{
}

void Fiber::Schedule() 
{
  if (scheduler_ == nullptr)
  {
    Run();
  }
  else 
  {
    asio::post(strand_, [this]
    {
      Run();
    });
  }
}

void Fiber::Run() 
{
  currentFiber = this;

  if (!coroutine_.IsCompleted())
  {
    coroutine_.Resume();
  }

  currentFiber = nullptr;

  if (coroutine_.IsCompleted())
  {
    Destroy();
  }
  else
  {
    Schedule();
  }
}

void Fiber::Suspend()
{
  coro::Coroutine::Suspend();
}

void Fiber::SleepFor(Millis delay)
{
  asio::steady_timer timer(scheduler_->get_executor());
  timer.expires_after(delay);

  timer.async_wait([this](const asio::error_code&) {
      coroutine_.Resume();
  });

  coro::Coroutine::Suspend();
}

Fiber* Fiber::Self() 
{
  return currentFiber;
}

Scheduler* Fiber::GetSelfScheduler()
{
  return Self()->scheduler_;
}

void Fiber::Destroy()
{
  delete this;
}

}  // namespace exe::fibers
