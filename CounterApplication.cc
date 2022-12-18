//
// Created by Rahul  Kushwaha on 12/17/22.
//

#include "CounterApplication.h"
#include "CounterEntry.pb.h"
#include <iostream>

namespace rk::project::counter {

CounterApplication::CounterApplication(std::shared_ptr<DurableLog> log,
                                       std::shared_ptr<Counter> counter)
    : log_{std::move(log)}, counter_{std::move(counter)} {}

/* static */ std::string CounterApplication::serialize(std::int64_t val) {
  CounterLogEntry entry;
  entry.set_commandtype(CounterLogEntry_CommandType::CounterLogEntry_CommandType_INCR);
  entry.set_description("Hello World");
  entry.set_val(val);

  return entry.SerializeAsString();
}

std::int64_t CounterApplication::increment(std::int64_t val) {
  log_->append(CounterApplication::serialize(val));
  return counter_->get();
}

std::int64_t CounterApplication::get() {
  return counter_->get();
}

}