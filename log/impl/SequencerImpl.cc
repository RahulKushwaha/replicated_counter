//
// Created by Rahul  Kushwaha on 12/31/22.
//

#include "SequencerImpl.h"
#include <folly/futures/Retrying.h>
#include "../utils/FutureUtils.h"

namespace rk::projects::durable_log {

namespace {
std::size_t constexpr MAXIMUM_SEQUENCER_RETIRES = 0;
}

SequencerImpl::SequencerImpl(
    std::string id,
    std::vector<std::shared_ptr<Replica>> replicaSet, LogId seedSeqNum) :
    id_{std::move(id)},
    replicaSet_{std::move(replicaSet)}, sequenceNum_{seedSeqNum},
    quorumSize_{(std::int32_t) replicaSet_.size() / 2 + 1} {
  LOG(INFO) << "Sequencer Initialized with seed: " << sequenceNum_;
}

std::string SequencerImpl::getId() {
  return id_;
}

folly::SemiFuture<LogId> SequencerImpl::append(std::string logEntryPayload) {
  LogId logId = sequenceNum_++;

  // Forever retrying future.
  return folly::futures::retrying(
      [maxRetries = MAXIMUM_SEQUENCER_RETIRES](
          std::size_t count,
          const folly::exception_wrapper &) {
        if (count < maxRetries) {
          return folly::makeSemiFuture(true);
        } else {
          return folly::makeSemiFuture(false);
        }
      },
      [this, logId, logEntryPayload](std::size_t) {
        std::vector<folly::SemiFuture<folly::Unit>>
            futures;
        for (auto &replica: replicaSet_) {
          folly::SemiFuture<folly::Unit>
              future = replica->append(logId, logEntryPayload);
          futures.emplace_back(std::move(future));
        }

        return utils::anyNSuccessful(std::move(futures), quorumSize_)
            .via(&folly::InlineExecutor::instance())
            .then([logId](folly::Try<folly::Unit> &&result) {
              if (result.hasException()) {
                LOG(ERROR) << result.exception().what();
                auto *exception =
                    result.template tryGetExceptionObject<utils::MultipleExceptions>();
                if (exception) {
                  for (auto &[index, ex]: exception->exceptions()) {
                    try {
                      std::rethrow_exception(ex);
                    } catch (const std::exception &e) {
                      LOG(ERROR) << "Future Index: " << index << " "
                                 << e.what();
                    }
                  }
                }

                std::rethrow_exception(result.exception().to_exception_ptr());
              }

              return logId;
            });
      });
}

}