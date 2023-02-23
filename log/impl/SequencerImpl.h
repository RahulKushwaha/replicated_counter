//
// Created by Rahul  Kushwaha on 12/31/22.
//

#pragma once
#include "../include/Sequencer.h"
#include "../include/Replica.h"

#include <vector>

namespace rk::projects::durable_log {

class SequencerImpl: public Sequencer {
 public:
  explicit SequencerImpl(std::string id,
                         std::vector<std::shared_ptr<Replica>> replicaSet,
                         LogId seedSeqNum,
                         bool isAlive = false);

 public:
  std::string getId() override;
  folly::SemiFuture<LogId> append(std::string logEntryPayload) override;
  folly::SemiFuture<LogId> latestAppendPosition() override;
  void start() override;
  bool isAlive() override;
  void kill() override;
  ~SequencerImpl() override = default;

 private:
  std::string id_;
  std::vector<std::shared_ptr<Replica>> replicaSet_;
  std::atomic<LogId> sequenceNum_;
  std::int32_t quorumSize_;
  bool isAlive_;
};

}