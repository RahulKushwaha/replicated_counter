//
// Created by Rahul  Kushwaha on 4/10/23.
//

#pragma once
#include "rocksdb/db.h"

namespace rk::projects::mydb {

class RocksDbFactory {
 public:
  struct RocksDbConfig {
    std::string path;
    bool createIfMissing;
  };

  static rocksdb::DB* provide(const RocksDbConfig& config) {
    rocksdb::DB* db;

    rocksdb::Options options;
    options.create_if_missing = true;

    rocksdb::Status status = rocksdb::DB::Open(options, config.path, &db);

    if (status.ok()) {
      return db;
    }

    return nullptr;
  }
};

}  // namespace rk::projects::mydb
