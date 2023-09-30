//
// Created by Rahul  Kushwaha on 9/14/23.
//

#pragma once
#include "KVStoreLite.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/checkpoint.h"
#include "rocksdb/utilities/optimistic_transaction_db.h"
#include "rocksdb/utilities/transaction_db.h"

namespace rk::projects::persistence {

class TransactionalRocksKVStoreLite;

class TxnManager {
 public:
  using txnId_t = std::uint64_t;

  explicit TxnManager(std::shared_ptr<rocksdb::OptimisticTransactionDB> txnDb)
      : txnDb_{std::move(txnDb)},
        txnLookup_{},
        mtx_{std::make_unique<std::mutex>()} {}

  folly::coro::Task<std::shared_ptr<TransactionalKVStoreLite>>
  createTxnHandle() {
    rocksdb::Transaction* txn =
        txnDb_->BeginTransaction(rocksdb::WriteOptions{});

    std::shared_ptr<rocksdb::Transaction> txnPtr{txn};
    auto txnStore = std::make_shared<TransactionalRocksKVStoreLite>(
        std::move(txnPtr), txnDb_);

    {
      std::lock_guard lg{*mtx_};
      txnLookup_.emplace(txn->GetId(), txnStore);
    }

    co_return txnStore;
  }

  folly::coro::Task<
      std::optional<std::shared_ptr<TransactionalRocksKVStoreLite>>>
  getTxnHandle(txnId_t txnId) {
    std::lock_guard lg{*mtx_};
    if (auto itr = txnLookup_.find(txnId); itr != txnLookup_.end()) {
      co_return {itr->second};
    }

    co_return {};
  }

 private:
  std::shared_ptr<rocksdb::OptimisticTransactionDB> txnDb_;
  std::map<txnId_t, std::shared_ptr<TransactionalRocksKVStoreLite>> txnLookup_;
  std::unique_ptr<std::mutex> mtx_;
};

class TransactionalRocksKVStoreLite : public TransactionalKVStoreLite {
 public:
  explicit TransactionalRocksKVStoreLite(
      std::shared_ptr<rocksdb::Transaction> txn,
      std::shared_ptr<rocksdb::OptimisticTransactionDB> txnDb)
      : txn_{std::move(txn)}, txnDb_{std::move(txnDb)} {}

  folly::coro::Task<std::uint64_t> getId() override { co_return txn_->GetId(); }

  folly::coro::Task<bool> put(std::string key, std::string value) override {
    auto status = txn_->Put(key, value);
    if (!status.ok()) {
      throw std::runtime_error{status.ToString()};
    }

    co_return true;
  }

  folly::coro::Task<bool> putIfNotExists(std::string key,
                                         std::string value) override {
    if (auto exists = (co_await get(key)).has_value()) {
      co_return false;
    }

    co_return co_await put(std::move(key), std::move(value));
  }

  folly::coro::Task<std::optional<std::string>> get(std::string key) override {
    std::string value;
    auto status = txn_->Get(rocksdb::ReadOptions{}, key, &value);
    if (!status.ok()) {
      throw std::runtime_error{status.ToString()};
    }

    co_return {value};
  }

  folly::coro::Task<bool> deleteRange(std::string start,
                                      std::string end) override {
    auto itr = txn_->GetIterator(rocksdb::ReadOptions{});
    rocksdb::Slice endSlice{end};

    itr->Seek(start);
    while (itr->Valid() && itr->key().compare(endSlice) < 0) {
      txn_->Delete(itr->key());
    }

    co_return true;
  }

  folly::coro::AsyncGenerator<KeyValue> scan(std::string prefix) override {
    auto itr = txn_->GetIterator(rocksdb::ReadOptions{});
    itr->Seek(prefix);
    while (itr->Valid() && itr->key().starts_with(prefix)) {
      auto r = std::make_tuple(itr->key().ToString(), itr->value().ToString());
      co_yield r;

      itr->Next();
    }
  }

  folly::coro::Task<bool> flushWal() override {
    auto status = txnDb_->FlushWAL(true);
    if (!status.ok()) {
      LOG(INFO) << "wal flush failed: " << status.ToString();
    }

    co_return status.ok();
  }

  folly::coro::Task<void> checkpoint(
      const std::string& checkpointDir) override {
    rocksdb::Checkpoint* checkpoint;
    auto status = rocksdb::Checkpoint::Create(txnDb_.get(), &checkpoint);
    if (!status.ok()) {
      LOG(INFO) << "failed to create checkpoint object" << status.ToString();
    }

    status = checkpoint->CreateCheckpoint(checkpointDir);

    if (!status.ok()) {
      LOG(INFO) << "failed to create checkpoint" << status.ToString();
    }

    co_return;
  }

  folly::coro::Task<bool> commit() override {
    auto status = txn_->Commit();
    if (status.ok()) {
      co_return true;
    }

    throw std::runtime_error{status.ToString()};
  }

 private:
  std::shared_ptr<rocksdb::Transaction> txn_;
  std::shared_ptr<rocksdb::OptimisticTransactionDB> txnDb_;
};

}  // namespace rk::projects::persistence