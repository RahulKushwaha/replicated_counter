//
// Created by Rahul  Kushwaha on 1/2/23.
//

#ifndef LOGSTORAGE_ORDEREDCOMPLETIONQUEUE_H
#define LOGSTORAGE_ORDEREDCOMPLETIONQUEUE_H

#include<cstdint>
#include <iostream>
#include <map>
#include <folly/futures/Future.h>

namespace rk::project::utils {

template<typename T>
class OrderedCompletionQueue {
 public:
  explicit OrderedCompletionQueue(std::int64_t startIndex = 0)
      : currentIndex_{startIndex} {}

  void add(std::int64_t index, folly::Promise<T> promise, T result) {
    PromiseState promiseState{std::move(promise), std::move(result)};
    promises_[index] = std::move(promiseState);

    auto startIndex = currentIndex_;
    std::vector<std::int64_t> toDelete;
    for (auto &[key, value]: promises_) {
      if (key == startIndex) {
        value.promise.setValue(value.result);
        toDelete.push_back(startIndex);
        startIndex++;
      } else {
        break;
      }
    }

    for (auto key: toDelete) {
      promises_.erase(key);
    }

    currentIndex_ = startIndex;
  }

  std::int64_t getCurrentIndex() {
    return currentIndex_;
  }

 private:
  struct PromiseState {
    folly::Promise<T> promise;
    T result;
  };

  std::int64_t currentIndex_;
  std::map<std::int64_t, PromiseState> promises_;
};

}
#endif //LOGSTORAGE_ORDEREDCOMPLETIONQUEUE_H
