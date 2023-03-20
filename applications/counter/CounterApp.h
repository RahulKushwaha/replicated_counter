//
// Created by Rahul  Kushwaha on 2/20/23.
//

#pragma once
#include <memory>
#include <unordered_map>

#include "log/include/VirtualLog.h"
#include "applications/counter/proto/CounterEntry.pb.h"

namespace rk::projects::counter_app {

using namespace rk::projects::durable_log;

class CounterApp {
 public:
  explicit CounterApp(std::shared_ptr<VirtualLog> virtualLog);

  std::int64_t incrementAndGet(std::string key, std::int64_t incrBy);
  std::int64_t decrementAndGet(std::string key, std::int64_t decrBy);
  std::int64_t getValue(std::string key);

 private:
  static std::string
  serialize(std::string key,
            std::int64_t val,
            CounterLogEntry_CommandType commandType);
  static CounterLogEntry deserialize(std::string payload);

  void apply(const CounterLogEntry &counterLogEntry);
  void sync(LogId to);

 private:
  std::shared_ptr<VirtualLog> virtualLog_;
  LogId lastAppliedEntry_;
  std::unique_ptr<std::mutex> mtx_;

  std::unordered_map<std::string, std::atomic_int64_t> lookup_;
};

}

