#include <exe/tp/thread_pool.hpp>
#include <twist/ed/local/ptr.hpp>
#include <wheels/core/panic.hpp>


namespace exe::tp {

twist::ed::ThreadLocalPtr<ThreadPool> ThreadPool::currentPool_ = nullptr;

ThreadPool::ThreadPool(size_t threads) 
  : stop_(false)
  , activeTaskCount_(0)
{
  workers_.reserve(threads);
}

void ThreadPool::Start() 
{
  for (size_t i = 0; i < workers_.capacity(); ++i) 
  {
    workers_.emplace_back([this]() 
    {
      while (true) 
      {
        auto task = tasks_.Take();

        if (!task)
        {
          break;
        }

        currentPool_ = this;

        (*task)();

        currentPool_ = nullptr;

        DecrementActiveTasks();
      }
    });
  }
}

ThreadPool::~ThreadPool() 
{
  assert(stop_ && "ThreadPool destructor called before Stop()");
}

void ThreadPool::Submit(Task task) 
{
  IncrementActiveTasks();
  tasks_.Put(std::move(task));
}

ThreadPool* ThreadPool::Current() 
{
  return currentPool_;
}

void ThreadPool::WaitIdle() 
{
  std::unique_lock<twist::ed::stdlike::mutex> lock(mutex_);

  cv_.wait(
    lock, 
    [this]() 
    { 
      return activeTaskCount_ == 0; 
    });
}

void ThreadPool::Stop() 
{
    {
      std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);
      stop_ = true;
    }

    tasks_.Close();

    for (auto& worker : workers_) 
    {
      worker.join();
    }
}

void ThreadPool::IncrementActiveTasks() 
{
  std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);

  ++activeTaskCount_;
}

void ThreadPool::DecrementActiveTasks() 
{
  std::lock_guard<twist::ed::stdlike::mutex> lock(mutex_);

  --activeTaskCount_;

  if (activeTaskCount_ == 0) 
  {
    cv_.notify_all();
  }
}

}  // namespace exe::tp
