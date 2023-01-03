//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "SequencerImpl.h"
#include "../utils/FutureUtils.h"

namespace rk::project::counter {

SequencerImpl::SequencerImpl(
    std::vector<std::shared_ptr<Replica>> replicaSet, LogId seedSeqNum) :
    replicaSet_{std::move(replicaSet)}, sequenceNum_{seedSeqNum},
    quorumSize_{(std::int32_t) replicaSet_.size() / 2 + 1} {
}

folly::SemiFuture<LogId> SequencerImpl::append(std::string logEntryPayload) {
  auto logId = sequenceNum_++;

  std::vector<folly::SemiFuture<folly::Unit>> futures;

  for (auto &replica: replicaSet_) {
    folly::SemiFuture<folly::Unit>
        future = replica->append(logId, logEntryPayload);
    futures.emplace_back(std::move(future));
  }

  auto result =
      rk::project::utils::anyNSuccessful(std::move(futures), quorumSize_);

  return std::move(result)
      .deferValue([logId](auto &&r) {
        return logId;
      });
}

}