//
// Created by Rahul  Kushwaha on 9/15/23.
//

#include "folly/experimental/coro/AsyncGenerator.h"
#include "gtest/gtest.h"
#include "persistence/TransactionalRocksKVStoreLite.h"
#include "persistence/tests/RocksTestFixture.h"

namespace rk::projects::persistence {

namespace {
folly::coro::Task<std::vector<std::tuple<std::string, std::string>>>
getKeyValues(folly::coro::AsyncGenerator<KVStoreLite::KeyValue> generator) {
  std::vector<KVStoreLite::KeyValue> output;
  while (auto item = co_await generator.next()) {
    output.emplace_back(*item);
  }

  co_return output;
}

}  // namespace

class TransactionalRocksKVStoreLiteTests : public RocksTestFixture {};

TEST_F(TransactionalRocksKVStoreLiteTests, createTxnHandle) {
  TxnManager txnManager{optimisticDb_};
  auto handle = txnManager.createTxnHandle().semi().get();
}

TEST_F(TransactionalRocksKVStoreLiteTests, getTxnHandle) {
  TxnManager txnManager{optimisticDb_};
  auto handle = txnManager.createTxnHandle().semi().get();

  ASSERT_TRUE(txnManager.getTxnHandle(handle->getId().semi().get())
                  .semi()
                  .get()
                  .has_value());
}

TEST_F(TransactionalRocksKVStoreLiteTests, putShowValueOnlyAfterCommit) {
  TxnManager txnManager{optimisticDb_};
  auto handle = txnManager.createTxnHandle().semi().get();
  handle->put("hello", "world").semi().get();

  {
    std::string value;
    auto status = db_->Get(rocksdb::ReadOptions{}, "hello", &value);
    ASSERT_FALSE(status.ok());
  }

  handle->commit().semi().get();

  {
    std::string value;
    auto status = db_->Get(rocksdb::ReadOptions{}, "hello", &value);
    ASSERT_TRUE(status.ok());
  }
}

TEST_F(TransactionalRocksKVStoreLiteTests, getReadYourOwnWrite) {
  TxnManager txnManager{optimisticDb_};
  auto handle = txnManager.createTxnHandle().semi().get();
  handle->put("hello", "world").semi().get();

  ASSERT_TRUE(handle->get("hello").semi().get());
}

TEST_F(TransactionalRocksKVStoreLiteTests, scan) {
  TxnManager txnManager{optimisticDb_};
  auto handle = txnManager.createTxnHandle().semi().get();

  std::vector<std::tuple<std::string, std::string>> keyValues;
  for (std::int32_t i = 0; i < 1000; i++) {
    auto key = fmt::format("key_{}", i);
    auto value = fmt::format("value_{}", i);
    keyValues.emplace_back(std::move(key), std::move(value));
  }

  std::sort(keyValues.begin(), keyValues.end());

  for (const auto& [key, value] : keyValues) {
    handle->put(key, value).semi().get();
  }

  auto response = getKeyValues(handle->scan()).semi().get();
  ASSERT_EQ(keyValues, response);
}

}  // namespace rk::projects::persistence