//
// Created by Rahul  Kushwaha on 12/17/22.
//

#ifndef LOGSTORAGE_SEQUENCER_H
#define LOGSTORAGE_SEQUENCER_H

#include <cstdint>
#include <atomic>

namespace rk::project::counter {

class Sequencer {
 public:
  std::int64_t getNextId();

  explicit Sequencer(std::int64_t seedVal = 0);
 private:
  std::atomic<std::int64_t> id_;
};

}

#endif //LOGSTORAGE_SEQUENCER_H
