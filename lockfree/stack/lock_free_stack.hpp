#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/ed/stdlike/atomic.hpp>

#include <cassert>
#include <optional>

// Treiber unbounded lock-free stack
// https://en.wikipedia.org/wiki/Treiber_stack

template <typename T>
class LockFreeStack 
{
  struct Node 
  {
    Node(T&& aValue) 
      : value(std::move(aValue))
      , next(StampedPtr<Node>{nullptr, 0})
      , innerCount(0) 
    {}

    T value;
    StampedPtr<Node> next;
    twist::ed::stdlike::atomic<int64_t> innerCount{0};
  };

 public:

  LockFreeStack() : top_(StampedPtr<Node>{nullptr, 0}) {}

  void Push(T item) 
  {
    StampedPtr<Node> newHead{new Node(std::move(item)), 0};

    newHead->next = top_.Load();

    while (!top_.CompareExchangeWeak(newHead->next, newHead))
    {}
  }

  std::optional<T> TryPop() 
  {
    while (true)
    {
      StampedPtr<Node> curTop;

      while (!top_.CompareExchangeWeak(curTop, curTop.IncrementStamp()))
      {} 

      if (!curTop)
      {
        return std::nullopt;
      }

      curTop = curTop.IncrementStamp();
      auto oldTop = curTop;

      if (top_.CompareExchangeWeak(curTop, curTop.raw_ptr->next))
      {
        assert(curTop);
        
        T value = std::move(curTop.raw_ptr->value);

        const int64_t stampIncrement = curTop.stamp - 1;

        if (curTop.raw_ptr->innerCount.fetch_add(stampIncrement) == -stampIncrement)
        {
          delete curTop.raw_ptr;
        }

        return value;
      }

      if (oldTop.raw_ptr->innerCount.fetch_sub(1) == 1)
      {
        delete oldTop.raw_ptr;
      }
    }
  }

  ~LockFreeStack() 
  {
    StampedPtr<Node> curTop = top_.Load();
    
    while (curTop)
    {
      StampedPtr<Node> newTop = curTop.raw_ptr->next;
      delete curTop.raw_ptr;
      curTop = newTop;
    }
  }

 private:
  AtomicStampedPtr<Node> top_;
};
