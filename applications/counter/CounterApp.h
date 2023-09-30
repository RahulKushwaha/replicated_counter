//
// Created by Rahul  Kushwaha on 2/20/23.
//

#pragma once
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/proto/CounterEntry.pb.h"
#include "folly/experimental/coro/Task.h"
#include "persistence/RocksKVStoreLite.h"

#include <memory>
#include <unordered_map>

namespace rk::projects::counter_app {
template <typename T>
using coro = folly::coro::Task<T>;

using namespace rk::projects::durable_log;

struct CounterKeyValue {
  std::string key;
  std::int64_t val;
};

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

  explicit CounterApp(std::shared_ptr<CounterAppStateMachine> stateMachine,
                      std::shared_ptr<persistence::KVStoreLite> kvStore);

  coro<std::int64_t> incrementAndGet(std::string key, std::int64_t incrBy);
  coro<std::int64_t> decrementAndGet(std::string key, std::int64_t decrBy);
  coro<std::int64_t> getValue(std::string key);
  coro<std::vector<CounterKeyValue>> batchUpdate(
      std::vector<Operation> operations);

  std::vector<CounterKeyValue> apply(const CounterLogEntries& counterLogEntries,
                                     durable_log::LogId logId);

  std::unordered_map<std::string, std::int64_t> getValues();

  coro<CounterAppSnapshot> snapshot(const std::string& snapshotDirectory);
  coro<std::optional<CounterAppSnapshot>> restoreFromSnapshot();

  LogId getLastSnapshotId() const;

 private:
  static CounterLogEntries serialize(const std::vector<Operation>& operations);

 private:
  std::shared_ptr<CounterAppStateMachine> stateMachine_;
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  LogId lastAppliedEntry_;
  LogId lastSnapshotId_;
  std::unique_ptr<std::mutex> mtx_;

  std::unordered_map<std::string, std::int64_t> lookup_;
};

}  // namespace rk::projects::counter_app
