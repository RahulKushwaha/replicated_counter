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
}  // namespace rk::projects::mydb::transaction
