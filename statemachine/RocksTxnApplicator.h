//
// Created by Rahul  Kushwaha on 6/10/23.
//
#pragma once
#include "rocksdb/db.h"
#include "statemachine/Common.h"
#include "statemachine/ConflictDetector.h"
#include "statemachine/include/StateMachine.h"
#include "statemachine/proto/RocksTxn.pb.h"
#include <set>

namespace rk::projects::state_machine {

class RocksTxnApplicator : public Applicator<RocksTxn, RocksTxnResult> {
public:
  explicit RocksTxnApplicator(
      std::shared_ptr<ConflictDetector> conflictDetector, rocksdb::DB *rocks)
      : conflictDetector_{std::move(conflictDetector)}, rocks_{rocks} {}

  folly::coro::Task<RocksTxnResult> apply(RocksTxn txn) override {
    RocksTxnResult txnResult{.txnSucceeded = false,
                             .speculativeTxnResult =
                                 SpeculativeTxnResult{.conflictDetected = false,
                                                      .txnSucceeded = false}};

    // fast path. check for speculative execution.
    // if the transaction has been speculatively executed, then we just
    // need to check for conflicts.
    if (txn.speculativeExecution.has_value()) {
      bool conflicted =
          conflictDetector_->conflicted(txn.speculativeExecution.value());

      // there are no conflicts. go straight ahead and write to database.
      if (!conflicted) {
        txnResult.speculativeTxnResult.conflictDetected = false;

        rocksdb::Status status = rocks_->Write(
            rocksdb::WriteOptions{},
            txn.speculativeExecution.value().batch.GetWriteBatch());

        if (status.ok()) {
          conflictDetector_->addKeySet(txn.logId,
                                       txn.speculativeExecution->keySet);

          txnResult.speculativeTxnResult.txnSucceeded = true;
        } else {
          txnResult.speculativeTxnResult.conflictDetected = true;
        }

        // We can return from here as the transaction did not have any conflict.
        co_return txnResult;
      }
    }

    // otherwise we need to re-execute the whole transaction.
    auto rockTxn1 = RocksTxn_1{};
    bool parseResult = rockTxn1.ParseFromString(txn.serializedPayload);
    assert(parseResult);

    std::vector<std::string> modifiedKeySet;
    rocksdb::WriteBatchWithIndex batch{};
    for (auto &k : rockTxn1.write_set()) {
      batch.Put(rocksdb::Slice{k.key()}, rocksdb::Slice{k.value()});
      modifiedKeySet.emplace_back(k.key());
    }

    for (auto &k : rockTxn1.delete_set()) {
      batch.Delete(rocksdb::Slice{k});
      modifiedKeySet.emplace_back(k);
    }

    auto status = rocks_->Write(rocksdb::WriteOptions{}, batch.GetWriteBatch());
    if (status.ok()) {
      conflictDetector_->addKeySet(txn.logId, modifiedKeySet);
      txnResult.txnSucceeded = true;
    }

    co_return txnResult;
  }

private:
  std::shared_ptr<ConflictDetector> conflictDetector_;
  rocksdb::DB *rocks_;
};

} // namespace rk::projects::state_machine
