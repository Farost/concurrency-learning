#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/ed/stdlike/atomic.hpp>

#include <optional>

// Treiber unbounded lock-free stack
// https://en.wikipedia.org/wiki/Treiber_stack

template <typename T>
class LockFreeStack {
  struct Node {
    Node(T&& aValue) : value(std::move(aValue)), next(StampedPtr<Node>{nullptr, 0}) {}

    T value;
    AtomicStampedPtr<Node> next;
    //int16_t innerCount{0};
  };

 public:

  LockFreeStack() : top_(StampedPtr<Node>{nullptr, 0}) {}
  void Push(T item) {
    Node* newHead = new Node;
    newHead->value = std::move(item);
    newHead->next = top_.Load();

    while (!top_.CompareExchangeWeak(newHead->next, newHead))
    {}
  }

  std::optional<T> TryPop() {
    StampedPtr<Node> oldTop = top_.Load();
    while (oldTop) {
      StampedPtr<Node> newTop = oldTop.DecrementStamp();
      if (top_.CompareExchangeWeak(oldTop, newTop)) {
        T value = std::move(oldTop.raw_ptr->value);
        if (oldTop.raw_ptr) {
          delete oldTop.raw_ptr;
        }
        return value;
      }
      oldTop = top_.Load();
    }
    return std::nullopt;
  }

  ~LockFreeStack() {
    while (TryPop())
    {}
  }

 private:
  AtomicStampedPtr<Node> top_;
};
