//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "VirtualLogImpl.h"
#include <random>

namespace rk::project::counter {

VirtualLogImpl::VirtualLogImpl(
    std::string id,
    std::string name,
    std::shared_ptr<Sequencer> sequencer,
    std::vector<std::shared_ptr<Replica>> replicaSet,
    std::shared_ptr<MetadataStore> metadataStore)
    :
    id_{std::move(id)},
    name_{std::move(name)},
    sequencer_{std::move(sequencer)},
    replicaSet_{std::move(replicaSet)},
    metadataStore_{std::move(metadataStore)} {
}

std::string VirtualLogImpl::getId() {
  return id_;
}

std::string VirtualLogImpl::getName() {
  return name_;
}

folly::SemiFuture<LogId> VirtualLogImpl::append(std::string logEntryPayload) {
  return sequencer_->append(logEntryPayload);
}

std::variant<LogEntry, LogReadError> VirtualLogImpl::getLogEntry(LogId logId) {
  return {LogReadError::NotFound};
}

void VirtualLogImpl::reconfigure() {
  // Make a copy of the replicaSet
  std::vector<std::shared_ptr<Replica>> replicaSet = replicaSet_;

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(replicaSet.begin(), replicaSet.end(), g);

  auto numberOfReplicas = replicaSet.size();
  auto majorityCount = numberOfReplicas / 2 + 1;
  replicaSet.resize(majorityCount);

  LogId minLogId = HighestNonExistingLogId, maxLogId = LowestNonExistingLogId;
  for (auto &replica: replicaSet) {
    auto endLogId = replica->seal();

    minLogId = std::min(minLogId, endLogId);
    maxLogId = std::max(maxLogId, endLogId);
  }

  // Re-replicate all the entries from minLogId to MaxLogId

}

}