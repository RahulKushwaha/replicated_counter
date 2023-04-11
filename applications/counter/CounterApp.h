//
// Created by Rahul  Kushwaha on 2/20/23.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "log/include/VirtualLog.h"
#include "applications/counter/proto/CounterEntry.pb.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::counter_app {

using namespace rk::projects::durable_log;

class CounterApp {
 public:
  struct IncrOperation {
    std::string key;
    std::int64_t incrBy;
  };

  struct DecrOperation {
    std::string key;
    std::int64_t incrBy;
  };

  struct CounterValue {
    std::string key;
    std::int64_t val;
  };
  using CounterValues = std::vector<CounterValue>;

  using Operation = std::variant<IncrOperation, DecrOperation>;

  explicit CounterApp(std::shared_ptr<VirtualLog> virtualLog);

  folly::coro::Task<std::int64_t>
  incrementAndGet(std::string key, std::int64_t incrBy);
  folly::coro::Task<std::int64_t>
  decrementAndGet(std::string key, std::int64_t decrBy);
  folly::coro::Task<std::int64_t> getValue(std::string key);
  folly::coro::Task<std::vector<CounterApp::CounterValue>>
  batchUptate(std::vector<Operation> operations);

 private:
  static std::string
  serialize(std::string key,
            std::int64_t val,
            CounterLogEntry_CommandType commandType);
  static CounterLogEnteries deserialize(const std::string& payload);

  static std::string
  serialize(const std::vector<Operation>& operations);

  void apply(const CounterLogEnteries &counterLogEnteries);
  std::vector<CounterValue> sync(LogId to);


 private:
  std::shared_ptr<VirtualLog> virtualLog_;
  LogId lastAppliedEntry_;
  std::unique_ptr<std::mutex> mtx_;

  std::unordered_map<std::string, std::atomic_int64_t> lookup_;
  CounterLogEnteries applyLogEntries(LogId logIdToApply);
};

}

