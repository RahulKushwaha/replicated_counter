//
// Created by Rahul  Kushwaha on 1/21/24.
//

#include "applications/mydb/backend/transaction/TwoPhaseCommit.h"

namespace rk::projects::mydb::transaction {

TwoPhaseCommit::TwoPhaseCommit(Transaction transaction,
                               std::shared_ptr<TxnManager> txnManager)
    : txnId_{transaction.id},
      transaction_{std::move(transaction)},
      txnManager_{std::move(txnManager)} {}

folly::coro::Task<void> TwoPhaseCommit::prepare() {
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

folly::coro::Task<void> TwoPhaseCommit::commit() {
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
  throw std::runtime_error{"METHOD_NOT_IMPLEMENTED"};
}

}  // namespace rk::projects::mydb::transaction
