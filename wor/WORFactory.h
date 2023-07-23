//
// Created by Rahul  Kushwaha on 7/8/23.
//
#pragma once

#include "log/proto/MetadataConfig.pb.h"
#include "wor/WriteOnceRegisterChainImpl.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

namespace rk::projects::wor {
struct PaxosWorConfig {
  durable_log::MetadataConfig metadataConfig;

  std::vector<durable_log::ReplicaConfig> getLocalConfig() const {
    std::vector<durable_log::ReplicaConfig> configs;
    for (const auto &config : metadataConfig.replica_set_config()) {
      if (config.local()) {
        configs.emplace_back(config);
      }
    }

    return configs;
  }

  std::vector<durable_log::ReplicaConfig> getRemoteConfigs() const {
    std::vector<durable_log::ReplicaConfig> configs;
    for (const auto &config : metadataConfig.replica_set_config()) {
      if (!config.local()) {
        configs.emplace_back(config);
      }
    }

    return configs;
  }

  std::int32_t totalMembers() const {
    return metadataConfig.replica_set_config_size();
  }
};

std::unique_ptr<WriteOnceRegister> makePaxosWor(WorId worId,
                                                PaxosWorConfig paxosWorConfig);
std::unique_ptr<WriteOnceRegisterChain> makeChainUsingInMemoryWor();
std::unique_ptr<WriteOnceRegisterChain> makeChainUsingPaxosWor();

} // namespace rk::projects::wor