#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> Fiber::currentFiber = nullptr;

Fiber::Fiber(Scheduler& scheduler, Routine routine) 
  : scheduler_(&scheduler)
  , routine_(std::move(routine))
  , coroutine_([&](){ routine_(); }) 
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
    scheduler_->Submit([this]
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

Fiber* Fiber::Self() 
{
  return currentFiber;
}

void Fiber::Destroy()
{
  delete this;
}

}  // namespace exe::fibers
