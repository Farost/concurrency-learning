#include <exe/fibers/sched/go.hpp>
#include <exe/fibers/core/fiber.hpp>

namespace exe::fibers {

void Go(Scheduler& scheduler, Routine routine) 
{
  Fiber* fiber = new Fiber(scheduler, std::move(routine));
  fiber->Schedule();
}

void Go(Routine routine) 
{
  exe::tp::ThreadPool* scheduler = exe::tp::ThreadPool::Current();
  
  if (scheduler != nullptr)
  {
    Go(*scheduler, std::move(routine));
  }
  else 
  {
    throw std::runtime_error("No scheduler available");
  }
}

}  // namespace exe::fibers
