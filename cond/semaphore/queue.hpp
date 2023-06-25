#pragma once

#include "tagged_semaphore.hpp"

#include <deque>
#include <optional>
#include <iostream>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
 public:
  explicit BoundedBlockingQueue(size_t capacity) 
    : dataSem_(capacity)
    , notEmptySem_(1)
    , putSem_(1)
    , takeSem_(1)
  {
    notEmptyToken_.emplace(notEmptySem_.Acquire());
  }

  void Put(T value) 
  {
    auto token = dataSem_.Acquire();

    auto putToken(putSem_.Acquire());

    data_.emplace_back(std::move(value));
    dataTokens_.emplace_back(std::move(token));

    if (notEmptyToken_.has_value())
    {
      auto notEmptyToken = std::move(*notEmptyToken_);
      notEmptyToken_ = std::nullopt;
      notEmptySem_.Release(std::move(notEmptyToken));
    }

    putSem_.Release(std::move(putToken));
  }

  T Take() 
  {
    auto takeToken(takeSem_.Acquire());

    WaitForData();

    auto token = std::move(dataTokens_.front());
    dataTokens_.pop_front();
    
    auto d = std::move(data_.front());
    data_.pop_front();

    dataSem_.Release(std::move(token));

    takeSem_.Release(std::move(takeToken));

    return d;
  }

 private:

  void WaitForData()
  {
    if (!data_.empty())
    {
      return;
    }

    auto token = notEmptySem_.Acquire();
    notEmptyToken_.emplace(std::move(token));
  }

 private:
  // Tags
  struct DataTag{};
  struct EmptyTag{};
  struct PutTag{};
  struct TakeTag{};

  using DataSemaphore = TaggedSemaphore<DataTag>;
  using NotEmptySemaphore = TaggedSemaphore<EmptyTag>;
  using PutSemaphore = TaggedSemaphore<PutTag>;
  using TakeSemaphore = TaggedSemaphore<TakeTag>;

 private:
  // Buffer
  std::deque<T> data_;

  DataSemaphore dataSem_;
  std::deque<typename DataSemaphore::Token> dataTokens_;

  NotEmptySemaphore notEmptySem_;
  std::optional<typename NotEmptySemaphore::Token> notEmptyToken_;

  PutSemaphore putSem_;
  TakeSemaphore takeSem_;
};
