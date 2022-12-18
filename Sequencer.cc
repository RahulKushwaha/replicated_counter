//
// Created by Rahul  Kushwaha on 12/17/22.
//

#include "Sequencer.h"

namespace rk::project::counter {

Sequencer::Sequencer(std::int64_t seedVal) {
  id_ = seedVal;
}

std::int64_t Sequencer::getNextId() {
  id_++;
  return id_;
}

}