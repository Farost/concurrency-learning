#include <exe/fibers/core/fiber.hpp>

#include <twist/ed/local/ptr.hpp>

#include <iostream>

namespace exe::fibers {

twist::ed::ThreadLocalPtr<Fiber> Fiber::currentFiber = nullptr;

Fiber::Fiber(tp::ThreadPool& threadPool, Routine routine) 
  : threadPool_(&threadPool)
  , routine_(std::move(routine))
  , coroutine_([&](){ routine_(); }) 
{
}

void Fiber::Schedule() 
{
  ////std::cout << "Schedule!" << this << std::endl;
  if (threadPool_ == nullptr)
  {
    Run();
  }
  else 
  {
    threadPool_->Submit([this]
    {
      Run();
    });
  }
}

void Fiber::Run() 
{
  //std::cout << "Run!" << this << std::endl;
  currentFiber = this;

  coroutine_.Resume();

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
  //std::cout << "Suspend!" << this << std::endl;
  //if (!coroutine_.IsCompleted())
  //{
    coro::Coroutine::Suspend();
  //}
  
}

Fiber* Fiber::Self() 
{
  return currentFiber;
}

void Fiber::Destroy()
{
  //std::cout << "Destroy!" << this << std::endl;
  delete this;
}

}  // namespace exe::fibers
