//
// Created by Rahul  Kushwaha on 2/20/23.
//

#pragma once
#include <memory>

#include "../../log/include/VirtualLog.h"
#include "CounterEntry.pb.h"

namespace rk::projects::counter_app {

using namespace rk::projects::durable_log;

class CounterApp {
 public:
  explicit CounterApp(std::shared_ptr<VirtualLog> virtualLog);

  std::int64_t incrementAndGet(std::int64_t incrBy);

  std::int64_t get();

 private:
  static std::string serialize(std::int64_t val);

  static CounterLogEntry deserialize(std::string payload);

  void apply(const CounterLogEntry &counterLogEntry);

 private:
  std::shared_ptr<VirtualLog> virtualLog_;
  LogId lastAppliedEntry_;

  std::atomic_int64_t val_;
};

}

