//
// Created by Rahul  Kushwaha on 2/20/23.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/proto/CounterEntry.pb.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::counter_app {
template <typename T> using coro = folly::coro::Task<T>;

struct CounterKeyValue {
  std::string key;
  std::int64_t val;
};

using namespace rk::projects::durable_log;

class CounterApp {
public:
  struct IncrOperation {
    std::string key;
    std::int64_t incrBy;
  };
  struct DecrOperation {
    std::string key;
    std::int64_t decrBy;
  };

  using Operation = std::variant<IncrOperation, DecrOperation>;

  explicit CounterApp(std::shared_ptr<CounterAppStateMachine> stateMachine);

  coro<std::int64_t> incrementAndGet(std::string key, std::int64_t incrBy);
  coro<std::int64_t> decrementAndGet(std::string key, std::int64_t decrBy);
  coro<std::int64_t> getValue(std::string key);
  coro<std::vector<CounterKeyValue>>
  batchUpdate(std::vector<Operation> operations);

  std::vector<CounterKeyValue>
  apply(const CounterLogEnteries &counterLogEntries);

private:
  static CounterLogEnteries serialize(const std::vector<Operation> &operations);

private:
  std::shared_ptr<CounterAppStateMachine> stateMachine_;
  LogId lastAppliedEntry_;
  std::unique_ptr<std::mutex> mtx_;

  std::unordered_map<std::string, std::atomic_int64_t> lookup_;
};

} // namespace rk::projects::counter_app
