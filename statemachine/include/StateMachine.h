//
// Created by Rahul  Kushwaha on 6/9/23.
//
#include "folly/experimental/coro/Task.h"

namespace rk::projects::state_machine {

template <typename T> class Applicator {
public:
  virtual folly::coro::Task<void> apply(T t) = 0;
};

template <typename T> class StateMachine {
public:
  virtual folly::coro::Task<void> append(T t) = 0;
  virtual void setApplicator(std::shared_ptr<Applicator<T>> applicator) = 0;
};

} // namespace rk::projects::state_machine
