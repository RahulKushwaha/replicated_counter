//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "SequencerImpl.h"

#include "log/utils/FutureUtils.h"

#include <folly/futures/Retrying.h>

namespace rk::projects::durable_log {

namespace {
std::size_t constexpr MAXIMUM_SEQUENCER_RETIRES = 0;
}

SequencerImpl::SequencerImpl(std::string id,
                             std::vector<std::shared_ptr<Replica>> replicaSet,
                             LogId seedSeqNum, VersionId versionId,
                             bool isAlive)
    : id_{std::move(id)},
      replicaSet_{std::move(replicaSet)},
      sequenceNum_{seedSeqNum},
      globalCommitIndex_{seedSeqNum},
      versionId_{versionId},
      quorumSize_{(std::int32_t)replicaSet_.size() / 2 + 1},
      isAlive_{isAlive},
      mtx_{std::make_unique<std::mutex>()} {
  LOG(INFO) << "Sequencer ID: " << id_
            << " Initialized with seed: " << sequenceNum_;
}

std::string SequencerImpl::getId() {
  return id_;
}

folly::SemiFuture<LogId> SequencerImpl::latestAppendPosition() {
  // Current sequenceNum_ always points to the next available entry.
  // Last occupied entry will be sequenceNum_ - 1;
  return folly::makeSemiFuture<LogId>(sequenceNum_ - 1);
}

folly::SemiFuture<LogId> SequencerImpl::append(std::string logEntryPayload) {
  LogId logId = sequenceNum_++;

  // Forever retrying future.
  return folly::futures::retrying(
      [maxRetries = MAXIMUM_SEQUENCER_RETIRES](
          std::size_t count, const folly::exception_wrapper&) {
        if (count < maxRetries) {
          return folly::makeSemiFuture(true);
        } else {
          return folly::makeSemiFuture(false);
        }
      },
      [this, logId, logEntryPayload](std::size_t) {
        std::vector<folly::SemiFuture<folly::Unit>> futures;
        for (auto& replica : replicaSet_) {
          folly::SemiFuture<folly::Unit> future =
              replica
                  ->append({globalCommitIndex_.load()}, versionId_, logId,
                           logEntryPayload)
                  .semi();
          futures.emplace_back(std::move(future));
        }

        return utils::anyNSuccessful(std::move(futures), quorumSize_)
            .via(&folly::InlineExecutor::instance())
            .then([logId, this](folly::Try<folly::Unit>&& result) {
              if (result.hasException()) {
                if (auto* exception = result.template tryGetExceptionObject<
                                      utils::MultipleExceptions>();
                    exception) {
                  LOG(ERROR) << exception->getDebugString();
                }

                std::rethrow_exception(result.exception().to_exception_ptr());
              }

              {
                std::lock_guard lg(*mtx_);
                if (globalCommitIndex_.load() < logId) {
                  globalCommitIndex_.store(logId);
                }
              }

              return logId;
            });
      });
}

void SequencerImpl::start(VersionId versionId, LogId sequenceNum) {
  sequenceNum_.store(sequenceNum);
  globalCommitIndex_.store(sequenceNum);
  versionId_ = versionId;
  isAlive_ = true;

  LOG(INFO) << "Sequencer ID: " << id_
            << " started with seed: " << sequenceNum_;
}

bool SequencerImpl::isAlive() {
  return isAlive_;
}

void SequencerImpl::stop() {
  isAlive_ = false;
}

}  // namespace rk::projects::durable_log