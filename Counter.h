//
// Created by Rahul  Kushwaha on 11/24/22.
//

#ifndef LOGSTORAGE_COUNTER_H
#define LOGSTORAGE_COUNTER_H

#include "StateMachine.h"
#include "DurableLog.h"
#include "CounterEntry.pb.h"

namespace rk::project::counter {

class Counter: public StateMachine<CounterLogEntry> {
 public:
  void apply(const CounterLogEntry &counterLogEntry) override;

  std::int64_t get();

  explicit Counter(long val,
                   std::shared_ptr<DurableLog> durableLog,
                   LogId lastAppliedPosition);

 private:
  std::atomic_long val_;
  std::shared_ptr<DurableLog> log_;
  LogId lastAppliedPosition_;
};

}


#endif //LOGSTORAGE_COUNTER_H
