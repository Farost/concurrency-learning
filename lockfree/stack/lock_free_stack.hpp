#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/ed/stdlike/atomic.hpp>

#include <optional>

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
    std::cout << "push" << std::endl;
    StampedPtr<Node> newHead{new Node(std::move(item)), 1};
    newHead.raw_ptr->innerCount.fetch_add(1);
    newHead->next = top_.Load();

    while (!top_.CompareExchangeWeak(newHead->next, newHead))
    {}
    
    std::cout << "PUSH: oldTop.stamp = " << top_.Load().stamp << " innerCount: " << top_.Load().raw_ptr->innerCount << std::endl;
        
    std::cout << "push successful" << std::endl;
  }

  std::optional<T> TryPop(bool unconditionally = false) 
  {
    std::cout << "pop" << std::endl;
    StampedPtr<Node> oldTop = top_.Load();

    while (oldTop)
    {
      auto innerCount = oldTop.raw_ptr->innerCount.load();
      std::cout << "real old top" << std::endl;
      StampedPtr<Node> newTop = oldTop.raw_ptr->next.IncrementStamp();

      if (top_.CompareExchangeWeak(oldTop, newTop)) 
      {
        T value = std::move(oldTop.raw_ptr->value);
        //oldTop.raw_ptr->innerCount.fetch_add(1);

        std::cout << "POP: oldTop.stamp = " << oldTop.stamp << " innerCount: " << oldTop.raw_ptr->innerCount << std::endl;
        if (unconditionally || oldTop.stamp == innerCount) 
        {
          delete oldTop.raw_ptr;
        }
      std::cout << "pop successful" << std::endl;
        return value;
      }
      std::cout << "another pop try" << std::endl;
    }

    return std::nullopt;
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

  StampedPtr<Node> LoadTop()
  {
    auto currentTop = top_.Load();
    while (!top_.CompareExchangeWeak(currentTop, currentTop.IncrementStamp()))
    {
    }

    return currentTop.IncrementStamp();
  }

 private:
  AtomicStampedPtr<Node> top_;
};
