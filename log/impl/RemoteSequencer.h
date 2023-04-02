//
// Created by Rahul  Kushwaha on 2/26/23.
//

#pragma once

#include "log/include/Sequencer.h"
#include "log/client/SequencerClient.h"

namespace rk::projects::durable_log {

class RemoteSequencer final: public Sequencer {
 public:
  explicit RemoteSequencer
      (std::shared_ptr<client::SequencerClient> sequencerClient)
      : sequencerClient_{std::move(sequencerClient)} {}

  std::string getId() override {
    return sequencerClient_->getId().get();
  }

  folly::SemiFuture<LogId> append(std::string logEntryPayload) override {
    return sequencerClient_->append(std::move(logEntryPayload));
  }

  folly::SemiFuture<LogId> latestAppendPosition() override {
    return sequencerClient_->latestAppendPosition();
  }

  void start(LogId sequenceNum) override {
    throw NotImplementedException{};
  }

  bool isAlive() override {
    return sequencerClient_->isAlive().get();
  }

  void stop() override {
    throw NotImplementedException{};
  }

  ~RemoteSequencer() override = default;

 private:
  std::shared_ptr<client::SequencerClient> sequencerClient_;
};

}
