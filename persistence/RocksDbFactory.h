//
// Created by Rahul  Kushwaha on 4/10/23.
//

#pragma once
#include "fmt/format.h"
#include "glog/logging.h"
#include "rocksdb/db.h"

namespace rk::projects::persistence {

class RocksDbFactory {
public:
  struct RocksDbConfig {
    std::string path;
    bool createIfMissing;
    bool manualWALFlush;
  };

  static rocksdb::DB *provide(const RocksDbConfig &config) {
    rocksdb::DB *db;

    rocksdb::Options options;
    options.create_if_missing = config.createIfMissing;
    options.manual_wal_flush = config.manualWALFlush;

    rocksdb::Status status = rocksdb::DB::Open(options, config.path, &db);

    if (status.ok()) {
      return db;
    }

    throw std::runtime_error{fmt::format(
        "rocks instance could not be created: {}", status.ToString())};
  }

  static std::shared_ptr<rocksdb::DB>
  provideSharedPtr(const RocksDbConfig &config) {
    auto *ptr = provide(config);
    return std::shared_ptr<rocksdb::DB>{ptr, [config](rocksdb::DB *db) {
                                          auto s = db->Close();
                                          LOG(INFO)
                                              << "db close: " << s.ToString();
                                        }};
  }
};

} // namespace rk::projects::persistence
