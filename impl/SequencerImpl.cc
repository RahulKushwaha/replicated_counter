//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "SequencerImpl.h"

namespace rk::project::counter {

SequencerImpl::SequencerImpl(
    std::vector<std::shared_ptr<Replica>> replicaSet, LogId seedSeqNum) :
    replicaSet_{std::move(replicaSet)}, sequenceNum_{seedSeqNum} {
}

LogId SequencerImpl::append(std::string logEntryPayload) {
  auto logId = sequenceNum_++;
  for (auto &replica: replicaSet_) {
    replica->append(logId, logEntryPayload);
  }

  return logId;
}

}