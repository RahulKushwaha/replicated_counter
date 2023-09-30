//
// Created by Rahul  Kushwaha on 4/10/23.
//

#pragma once
#include "fmt/format.h"
#include "glog/logging.h"
#include "rocksdb/db.h"
#include "rocksdb/utilities/optimistic_transaction_db.h"

namespace rk::projects::persistence {

class RocksDbFactory {
 public:
  struct RocksDbConfig {
    std::string path;
    bool createIfMissing;
    bool manualWALFlush;
    bool destroyIfExists;
  };

  static std::shared_ptr<rocksdb::DB> provideSharedPtr(
      const RocksDbConfig& config) {
    LOG(INFO) << config.path;
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = config.createIfMissing;
    options.manual_wal_flush = config.manualWALFlush;

    if (config.destroyIfExists) {
      auto destroyStatus = rocksdb::DestroyDB(config.path, options);
      LOG(INFO) << "destroying db: " << destroyStatus.ToString();
      assert(destroyStatus.ok());
    }

    rocksdb::Status status = rocksdb::DB::Open(options, config.path, &db);

    if (!status.ok()) {
      throw std::runtime_error{fmt::format(
          "rocks instance could not be created: {}", status.ToString())};
    }

    return std::shared_ptr<rocksdb::DB>{db, [config](rocksdb::DB* db) {
                                          auto s = db->Close();
                                          LOG(INFO)
                                              << "db close: " << s.ToString();
                                        }};
  }

  static std::shared_ptr<rocksdb::OptimisticTransactionDB> provideOptimisticDb(
      const RocksDbConfig& config) {
    LOG(INFO) << config.path;
    rocksdb::OptimisticTransactionDB* db;
    rocksdb::Options options;
    options.create_if_missing = config.createIfMissing;
    options.manual_wal_flush = config.manualWALFlush;

    if (config.destroyIfExists) {
      auto destroyStatus = rocksdb::DestroyDB(config.path, options);
      LOG(INFO) << "destroying db: " << destroyStatus.ToString();
      assert(destroyStatus.ok());
    }

    rocksdb::Status status =
        rocksdb::OptimisticTransactionDB::Open(options, config.path, &db);

    if (!status.ok()) {
      throw std::runtime_error{fmt::format(
          "rocks instance could not be created: {}", status.ToString())};
    }

    return std::shared_ptr<rocksdb::OptimisticTransactionDB>{
        db, [config](rocksdb::OptimisticTransactionDB* db) {
          auto s = db->Close();
          LOG(INFO) << "db close: " << s.ToString();
        }};
  }
};

}  // namespace rk::projects::persistence
