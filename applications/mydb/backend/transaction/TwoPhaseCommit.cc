//
// Created by Rahul  Kushwaha on 1/21/24.
//

#include "applications/mydb/backend/transaction/TwoPhaseCommit.h"

#include "folly/experimental/coro/Collect.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

TwoPhaseCommit::TwoPhaseCommit(Transaction transaction,
                               std::shared_ptr<TxnManager> txnManager,
                               std::shared_ptr<PartitionMap> partitionMap)
    : txnId_{transaction.id()},
      transaction_{std::move(transaction)},
      txnManager_{std::move(txnManager)},
      partitionMap_{std::move(partitionMap)} {}

folly::coro::Task<void> TwoPhaseCommit::prepare() {
  std::vector<folly::SemiFuture<client::PrepareTransactionResponse>> tasks;

  for (auto& [partitionId, request] : transaction_.partition_status()) {
    auto result = partitionMap_->getPartitionClient(partitionId);
    assert(result.has_value() &&
           "2pc: partitionClient not present for the partitionId");
    auto db = result.value();

    client::PrepareTransactionRequest prepareRequest{};
    prepareRequest.mutable_request()->CopyFrom(request);

    auto task = db->prepareTransaction(&prepareRequest);
    tasks.emplace_back(std::move(task).semi());
  }

  co_await folly::coro::collectAllRange(std::move(tasks));

  co_return;
}

folly::coro::Task<void> TwoPhaseCommit::commit() {
  // we get approval from all the partitions
  if (transaction_.total_votes() == transaction_.partition_status_size()) {

  }

  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

folly::coro::Task<void> TwoPhaseCommit::abort() {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

folly::coro::Task<TwoPhaseCommit::AbortOrCommitResult>
TwoPhaseCommit::abortOrCommit() {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

folly::coro::Task<Transaction> TwoPhaseCommit::getTxn() {
  co_return transaction_;
}

}  // namespace rk::projects::mydb::transaction
