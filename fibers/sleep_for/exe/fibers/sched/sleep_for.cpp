#include <exe/fibers/sched/sleep_for.hpp>

namespace exe::fibers {

void SleepFor(Millis delay) 
{
  Fiber* fiber = Fiber::Self();

  if (fiber != nullptr)
  {
    fiber->SleepFor(delay);
  }
  else 
  {
    throw std::runtime_error("Sleep for not in a fiber");
  }
}

}  // namespace exe::fibers
