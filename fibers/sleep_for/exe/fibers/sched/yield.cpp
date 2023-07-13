#include <exe/fibers/sched/yield.hpp>
#include <exe/fibers/core/fiber.hpp>

namespace exe::fibers {

void Yield() 
{
  Fiber* fiber = Fiber::Self();

  if (fiber != nullptr)
  {
    fiber->Suspend();
  }
  else 
  {
    throw std::runtime_error("Not in a fiber");
  }
}

}  // namespace exe::fibers
