//
// Created by Rahul  Kushwaha on 12/17/22.
//

#ifndef LOGSTORAGE_COUNTERAPPLICATION_H
#define LOGSTORAGE_COUNTERAPPLICATION_H

#include <cstdint>
#include <memory>
#include "DurableLog.h"
#include "Counter.h"

namespace rk::project::counter {

class CounterApplication {
 public:
  std::int64_t increment(std::int64_t val = 1);
  std::int64_t get();

  explicit CounterApplication(std::shared_ptr<DurableLog> log,
                              std::shared_ptr<Counter> counter);
 private:
  std::shared_ptr<DurableLog> log_;
  std::shared_ptr<Counter> counter_;

  static std::string serialize(std::int64_t val);
};

}

#endif //LOGSTORAGE_COUNTERAPPLICATION_H
