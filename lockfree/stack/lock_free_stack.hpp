#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/ed/stdlike/atomic.hpp>

#include <optional>

#include <cassert>
#include <iostream>

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
    twist::ed::stdlike::atomic<uint64_t> innerCount{0};
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
      StampedPtr<Node> oldTop;

      while (!top_.CompareExchangeWeak(oldTop, oldTop.IncrementStamp()))
      {} // 1, 2

      if (!oldTop)
      {
        return std::nullopt;
      }

      oldTop = oldTop.IncrementStamp(); // 1, 2

      if (top_.CompareExchangeWeak(oldTop, oldTop.raw_ptr->next)) // 1, 2
      {
        assert(oldTop);
        
        T value = std::move(oldTop.raw_ptr->value);
        while (true)
        {
          auto innerCount = oldTop.raw_ptr->innerCount.load();
          if (oldTop.raw_ptr->innerCount.compare_exchange_weak(innerCount, innerCount + 1))
          {
            break;
          }
        }

        assert(oldTop);

        uint64_t innerCount = oldTop.raw_ptr->innerCount.fetch_add(1);
        
        if (oldTop.stamp == innerCount)
        {
          delete oldTop.raw_ptr;
        }

        return value;
      }
      
      assert(oldTop);
      uint64_t innerCount = oldTop.raw_ptr->innerCount.fetch_add(1);
      
      assert(oldTop.stamp >= innerCount);
      
      if (oldTop.stamp == innerCount)
      {
        delete oldTop.raw_ptr;
      }
    }
  }

  ~LockFreeStack() 
  {
    std::cout << "Destructor" << std::endl;
    StampedPtr<Node> oldTop = top_.Load();
    while (oldTop)
    {
      StampedPtr<Node> newTop = oldTop.raw_ptr->next;
      delete oldTop.raw_ptr;
      oldTop = newTop;
    }
  }

 private:
  AtomicStampedPtr<Node> top_;
};
