#include <exe/coro/core.hpp>

#include <twist/ed/local/ptr.hpp>

#include <wheels/core/assert.hpp>
#include <wheels/core/compiler.hpp>

namespace exe::coro {

Coroutine* Coroutine::currentCoroutine = nullptr;

static const size_t kDefaultStackSize = 64'000'000;

Coroutine::Coroutine(Routine routine) 
  : routine_(std::move(routine))
  , stack_(sure::Stack::AllocateBytes(kDefaultStackSize))
  , exception_()
  , completed_(false)
{
  context_.Setup(stack_.MutView(), this);
}

void Coroutine::Resume() 
{
  if (completed_)
  {
    return;
  }

  Coroutine* callerCoroutine = std::exchange(currentCoroutine, this);

  outerContext_.SwitchTo(context_);

  currentCoroutine = callerCoroutine;

  if (exception_)
  {
    std::rethrow_exception(exception_);
  }
}

void Coroutine::Suspend() 
{
  currentCoroutine->SuspendInternal();
}

bool Coroutine::IsCompleted() const 
{
  return completed_;
}

void Coroutine::Run() noexcept 
{
  try
  {
    routine_();
  }
  catch(...)
  {
    exception_ = std::current_exception();
  }

  completed_ = true;
  context_.ExitTo(outerContext_);
}

void Coroutine::SuspendInternal() 
{
  context_.SwitchTo(outerContext_);
}

}  // namespace exe::coro
