//
// Created by Rahul  Kushwaha on 1/14/24.
//

#pragma once
#include "applications/mydb/backend/PartitionMap.h"
#include "applications/mydb/backend/transaction/TxnManager.h"
#include "applications/mydb/client/proto/db.pb.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

template <typename T>
using coro = folly::coro::Task<T>;

class TxnCoordinator {
 public:
  coro<client::MultiTableOperationResponse> multiTableOperation(
      const client::MultiTableOperationRequest* request);

 private:
  std::shared_ptr<PartitionMap> partitionMap_;
  std::shared_ptr<TxnManager> txnManager_;
};

}  // namespace rk::projects::mydb::transaction