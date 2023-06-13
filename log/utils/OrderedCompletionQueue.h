//
// Created by Rahul  Kushwaha on 1/2/23.
//

#pragma once

#include <cstdint>
#include <folly/futures/Future.h>
#include <iostream>
#include <map>

namespace rk::projects::utils {

template <typename T> class OrderedCompletionQueue {
public:
  explicit OrderedCompletionQueue(std::int64_t startIndex = 0)
      : currentIndex_{startIndex} {}

  void completeAllBelow(std::int64_t index) {
    std::vector<std::int64_t> toDelete;

    for (auto &[key, value] : promises_) {
      if (key <= index) {
        value->promise.setValue(value->result);
        toDelete.push_back(key);
      }
    }

    for (auto key : toDelete) {
      promises_.erase(key);
    }

    if (index > currentIndex_) {
      currentIndex_ = index;
    }

    checkAndCompleteAfterCurrentIndex();
  }

  /*
   * Add a promise with its result at an index. All the promises between
   * currentIndex and first hole will be completed.
   *
   * currentIndex = 4
   * [4]HOLE, 5, HOLE, 7, 8, 9
   * adding 4 complete 4, 5
   *
   * currentIndex = 4
   * [4]HOLE, 5, HOLE, 7, 8, 9
   * adding 10 does not complete any promise.
   *
   * All promises from the currentIndex to the first hole are completed.
   * */
  void add(std::int64_t index, folly::Promise<T> promise, T result) {
    promises_[index] = std::make_shared<PromiseState>(
        PromiseState{std::move(promise), std::move(result)});
    checkAndCompleteAfterCurrentIndex();
  }

  std::int64_t getCurrentIndex() { return currentIndex_; }

private:
  void checkAndCompleteAfterCurrentIndex() {
    auto startIndex = currentIndex_;
    std::vector<std::int64_t> toDelete;
    for (auto &[key, value] : promises_) {
      if (key == startIndex) {
        value->promise.setValue(value->result);
        toDelete.push_back(startIndex);
        startIndex++;
      } else {
        break;
      }
    }

    for (auto key : toDelete) {
      promises_.erase(key);
    }

    currentIndex_ = startIndex;
  }

private:
  struct PromiseState {
    folly::Promise<T> promise;
    T result;
  };

  std::int64_t currentIndex_;
  std::map<std::int64_t, std::shared_ptr<PromiseState>> promises_;
};

} // namespace rk::projects::utils
