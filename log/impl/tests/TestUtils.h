//
// Created by Rahul  Kushwaha on 1/16/23.
//

#pragma once

#include "MockReplica.h"
#include "log/impl/InMemoryMetadataStore.h"
#include "log/impl/NanoLogStoreImpl.h"
#include "log/impl/RegistryImpl.h"
#include "log/impl/ReplicaImpl.h"
#include "log/impl/SequencerImpl.h"
#include "log/include/Registry.h"
#include "log/utils/UuidGenerator.h"
#include <gtest/gtest.h>

namespace rk::projects::durable_log {
using namespace testing;

struct SequencerCreationResult {
  std::shared_ptr<Sequencer> sequencer;
  std::vector<std::shared_ptr<Replica>> replicaSet;
  std::vector<std::shared_ptr<Replica>> badReplicaSet;
  std::vector<std::shared_ptr<Replica>> goodReplicaSet;
  std::shared_ptr<MetadataStore> metadataStore;
  MetadataConfig initialMetadataConfig;
  std::shared_ptr<Registry> registry;
};

inline SequencerCreationResult
createSequencer(std::int32_t numberOfBadReplicas = 0,
                std::int64_t sequencerStartNum = 1) {
  std::shared_ptr<Registry> registry = std::make_shared<RegistryImpl>();
  std::shared_ptr<MetadataStore> metadataStore =
      std::make_shared<InMemoryMetadataStore>();

  // Add metadata block.
  MetadataConfig config;
  config.set_version_id(1);
  config.set_previous_version_id(0);
  config.set_start_index(1);
  config.set_end_index(1000);

  std::int32_t totalNumberOfReplicas = 5;
  std::vector<std::shared_ptr<Replica>> replicaSet;
  std::vector<std::shared_ptr<Replica>> badReplicaSet;
  std::vector<std::shared_ptr<Replica>> goodReplicaSet;

  for (std::int32_t i = 0; i < numberOfBadReplicas; i++) {
    std::shared_ptr<MockReplica> mockReplica = std::make_shared<MockReplica>();
    ON_CALL(*mockReplica, getId()).WillByDefault([]() {
      return "MOCK_REPLICA_" + utils::UuidGenerator::instance().generate();
    });

    ON_CALL(*mockReplica, append(_, _, _, _, _)).WillByDefault([]() {
      return folly::coro::makeErrorTask<folly::Unit>(
          folly::make_exception_wrapper<NonRecoverableError>(
              NonRecoverableError{}));
    });

    ON_CALL(*mockReplica, getLogEntry(_, _)).WillByDefault([]() {
      return folly::coro::makeErrorTask<std::variant<LogEntry, LogReadError>>(
          folly::make_exception_wrapper<NonRecoverableError>(
              NonRecoverableError{}));
    });

    config.mutable_replica_set_config()->Add()->set_id(mockReplica->getId());
    registry->registerReplica(mockReplica);
    badReplicaSet.emplace_back(mockReplica);
    replicaSet.emplace_back(std::move(mockReplica));
  }

  for (std::int32_t i = 0; i < totalNumberOfReplicas - numberOfBadReplicas;
       i++) {
    auto nanoLogFactory = std::make_shared<NanoLogFactory>(
        persistence::RocksDbFactory::RocksDbConfig{});

    std::shared_ptr<Replica> replica = std::make_shared<ReplicaImpl>(
        "REPLICA_" + utils::UuidGenerator::instance().generate(), "random_name",
        std::make_shared<NanoLogStoreImpl>(), metadataStore,
        std::move(nanoLogFactory), NanoLogType::InMemory);

    config.mutable_replica_set_config()->Add()->set_id(replica->getId());
    registry->registerReplica(replica);
    goodReplicaSet.emplace_back(replica);
    replicaSet.emplace_back(std::move(replica));
  }

  std::shared_ptr<Sequencer> sequencer = std::make_shared<SequencerImpl>(
      "SEQUENCER_" + utils::UuidGenerator::instance().generate(), replicaSet,
      sequencerStartNum, config.version_id());
  registry->registerSequencer(sequencer);

  config.mutable_sequencer_config()->set_id(sequencer->getId());

  metadataStore->compareAndAppendRange(0, config);

  return {sequencer,     replicaSet, badReplicaSet, goodReplicaSet,
          metadataStore, config,     registry};
}

} // namespace rk::projects::durable_log