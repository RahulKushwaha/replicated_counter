//
// Created by Rahul  Kushwaha on 6/9/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"

namespace rk::projects::state_machine {

template <typename T, typename R> class Applicator {
public:
  virtual folly::coro::Task<R> apply(T t) = 0;
};

template <typename T, typename R> class StateMachine {
public:
  virtual folly::coro::Task<R> append(T t) = 0;
  virtual void setApplicator(std::shared_ptr<Applicator<T, R>> applicator) = 0;
};

} // namespace rk::projects::state_machine
