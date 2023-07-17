//
// Created by Rahul  Kushwaha on 6/9/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "variant"

namespace rk::projects::state_machine {

template <typename T> using coro = folly::coro::Task<T>;
template <typename... T> using ReturnType = std::variant<T...>;

using LogPosition = std::int64_t;

template <typename T, typename R> class Applicator {
public:
  virtual coro<R> apply(T t) = 0;
};

template <typename T, typename R> class StateMachine {
public:
  virtual coro<R> append(T t) = 0;
  virtual coro<R> apply(T t) = 0;
  virtual coro<void> sync() { co_return; }
  virtual void setApplicator(std::shared_ptr<Applicator<T, R>> applicator) = 0;

  virtual void setUpstreamStateMachine(
      std::shared_ptr<StateMachine<T, R>> upstreamStateMachine) {}
};

} // namespace rk::projects::state_machine
