//
// Created by Rahul  Kushwaha on 6/25/23.
//
#pragma once

#include "KVStoreLite.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/write_batch_with_index.h"

namespace rk::projects::persistence {

class RocksKVStoreLite : public KVStoreLite {
public:
  explicit RocksKVStoreLite(std::shared_ptr<rocksdb::DB> rocks)
      : rocks_{std::move(rocks)} {}

  folly::coro::Task<bool> put(std::string key, std::string value) override {
    rocksdb::WriteBatchWithIndex batch{};
    batch.Put(key, value);

    rocksdb::Status status =
        rocks_->Write(rocksdb::WriteOptions{}, batch.GetWriteBatch());

    co_return status.ok();
  }

  folly::coro::Task<std::optional<std::string>> get(std::string key) override {
    std::string value{};
    rocksdb::Status status = rocks_->Get(rocksdb::ReadOptions{}, key, &value);

    if (status.ok()) {
      co_return value;
    }

    if (status.IsNotFound()) {
      co_return {};
    }

    throw std::runtime_error{status.ToString()};
  }

  folly::coro::Task<bool> putIfNotExists(std::string key,
                                         std::string value) override {
    auto getResult = co_await get(key);
    if (getResult.has_value()) {
      co_return false;
    }

    co_return co_await put(key, value);
  }

  folly::coro::Task<bool> deleteRange(std::string start,
                                      std::string end) override {
    rocksdb::Status status =
        rocks_->DeleteRange(rocksdb::WriteOptions{}, nullptr, start, end);

    if (status.ok()) {
      LOG(INFO) << "failed to delete range";
      co_return false;
    }

    co_return true;
  }

  folly::coro::AsyncGenerator<KeyValue> scan(std::string prefix) override {
    auto itr = rocks_->NewIterator(rocksdb::ReadOptions{});
    itr->Seek(prefix);
    while (itr->Valid() && itr->key().starts_with(prefix)) {
      auto r = std::make_tuple(itr->key().ToString(), itr->value().ToString());
      co_yield r;

      itr->Next();
    }
  }

  folly::coro::Task<bool> flushWal() override {
    auto s = rocks_->FlushWAL(true);
    if (!s.ok()) {
      LOG(INFO) << "wal flush failed: " << s.ToString();
    }

    co_return s.ok();
  }

private:
  std::shared_ptr<rocksdb::DB> rocks_;
};

} // namespace rk::projects::persistence