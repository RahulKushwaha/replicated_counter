//
// Created by Rahul  Kushwaha on 1/16/24.
//

#pragma once
#include "applications/mydb/backend/transaction/Models.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::mydb::transaction {

class TxnManager {
 public:
  folly::coro::Task<Transaction> get(std::string transactionId);
  folly::coro::Task<bool> create(Transaction transaction);
  folly::coro::Task<bool> update(Transaction transaction);
  folly::coro::Task<bool> del(std::string transactionId);
};

}  // namespace rk::projects::mydb::transaction