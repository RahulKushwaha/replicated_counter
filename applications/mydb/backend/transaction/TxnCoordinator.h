//
// Created by Rahul  Kushwaha on 1/14/24.
//

#pragma once
#include "applications/mydb/backend/PartitionMap.h"
#include "applications/mydb/backend/transaction/TxnManager.h"
#include "applications/mydb/backend/transaction/proto/txn.pb.h"
#include "applications/mydb/include/Db.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

class TxnCoordinator {
 public:
  coro<client::MultiTableOperationResponse> multiTableOperation(
      const client::MultiTableOperationRequest* request);

 private:
  std::shared_ptr<PartitionMap> partitionMap_;
  std::shared_ptr<TxnManager> txnManager_;
};

}  // namespace rk::projects::mydb::transaction