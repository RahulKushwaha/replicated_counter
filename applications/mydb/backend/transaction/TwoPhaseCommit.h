//
// Created by Rahul  Kushwaha on 1/21/24.
//
#pragma once
#include "applications/mydb/backend/PartitionMap.h"
#include "applications/mydb/backend/transaction/Models.h"
#include "applications/mydb/backend/transaction/TxnManager.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

class TwoPhaseCommit {
 public:
  enum class AbortOrCommitResult {
    Abort,
    Commit,
  };

  TwoPhaseCommit(Transaction transaction,
                 std::shared_ptr<TxnManager> txnManager,
                 std::shared_ptr<PartitionMap> partitionMap);

  folly::coro::Task<void> prepare();
  folly::coro::Task<void> commit();
  folly::coro::Task<void> abort();

  folly::coro::Task<AbortOrCommitResult> abortOrCommit();

  folly::coro::Task<Transaction> getTxn();

 private:
  std::string txnId_;
  Transaction transaction_;
  std::shared_ptr<TxnManager> txnManager_;
  std::shared_ptr<PartitionMap> partitionMap_;
};

}  // namespace rk::projects::mydb::transaction