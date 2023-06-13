//
// Created by Rahul  Kushwaha on 3/19/23.
//

#pragma once

#include <cstdint>
#include <string>

namespace rk::projects::durable_log {

struct EnsembleNodeConfig {
  std::string sequencerId;
  std::int32_t sequencerPort;

  std::string replicaId;
  std::int32_t replicaPort;

  std::int32_t metadataStorePort;
};

} // namespace rk::projects::durable_log
