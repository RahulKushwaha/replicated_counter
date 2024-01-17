//
// Created by Rahul  Kushwaha on 1/16/24.
//

#include "applications/mydb/backend/transaction/TxnManager.h"

namespace rk::projects::mydb::transaction {

folly::coro::Task<Transaction> TxnManager::get(std::string transactionId) {}

folly::coro::Task<bool> TxnManager::create(Transaction transaction) {}

folly::coro::Task<bool> TxnManager::update(Transaction transaction) {}

folly::coro::Task<bool> TxnManager::del(std::string transactionId) {}

}  // namespace rk::projects::mydb::transaction