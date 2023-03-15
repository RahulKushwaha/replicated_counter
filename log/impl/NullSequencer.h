//
// Created by Rahul  Kushwaha on 3/14/23.
//

#pragma once

#include "log/include/Sequencer.h"

namespace rk::projects::durable_log {

namespace {
constexpr char *NULL_SEQUENCER_ID = "NULL_SEQUENCER";
}

/* NullSequencer is a special kind of Sequencer that throws exception
 * for any kind of action.
 * */
class NullSequencer: public Sequencer {
 public:
  std::string getId() override {
    return std::string{NULL_SEQUENCER_ID};
  }

  folly::SemiFuture<LogId> append(std::string logEntryPayload) override {
    throw NullSequencerException{};
  }

  folly::SemiFuture<LogId> latestAppendPosition() override {
    throw NullSequencerException{};
  }

  void start() override {
    throw NullSequencerException{};
  }

  bool isAlive() override {
    return false;
  }

  void kill() override {
    throw NullSequencerException{};
  }

  ~NullSequencer() override = default;
};

}