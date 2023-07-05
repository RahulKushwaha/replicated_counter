//
// Created by Rahul  Kushwaha on 7/4/23.
//

#pragma once
#include "log/impl/InMemoryNanoLog.h"
#include "log/impl/RocksNanoLog.h"
#include "log/include/Common.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"

namespace rk::projects::durable_log {

enum class NanoLogType {
  InMemory,
  Rocks,
};

class NanoLogFactory {
public:
  explicit NanoLogFactory(
      persistence::RocksDbFactory::RocksDbConfig rocksConfig)
      : rocksConfig_{std::move(rocksConfig)} {}

  coro<std::shared_ptr<NanoLog>> makeNanoLog(NanoLogType nanoLogType,
                                             std::string id, std::string name,
                                             std::string metadataVersionId,
                                             LogId startIndex, LogId endIndex,
                                             bool sealed) {
    switch (nanoLogType) {
    case NanoLogType::InMemory:
      co_return std::make_shared<InMemoryNanoLog>(
          std::move(id), std::move(name), std::move(metadataVersionId),
          startIndex, endIndex, sealed);

    case NanoLogType::Rocks:
      auto rocks = persistence::RocksDbFactory::provideSharedPtr(rocksConfig_);
      auto kvStore =
          std::make_shared<persistence::RocksKVStoreLite>(std::move(rocks));
      auto rocksNanoLog = std::make_shared<RocksNanoLog>(
          std::move(id), std::move(name), std::move(metadataVersionId),
          startIndex, endIndex, sealed, std::move(kvStore));
      co_await rocksNanoLog->init();

      co_return rocksNanoLog;
    }

    throw std::runtime_error{"unknown nanolog type"};
  }

private:
  persistence::RocksDbFactory::RocksDbConfig rocksConfig_;
};

} // namespace rk::projects::durable_log
