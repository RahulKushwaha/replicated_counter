//
// Created by Rahul  Kushwaha on 1/16/23.
//

#pragma once

#include <gtest/gtest.h>
#include "../SequencerImpl.h"
#include "../ReplicaImpl.h"
#include "../InMemoryMetadataStore.h"
#include "../NanoLogStoreImpl.h"
#include "MockReplica.h"

namespace rk::project::counter {
using namespace testing;

struct SequencerCreationResult {
  std::shared_ptr<Sequencer> sequencer;
  std::vector<std::shared_ptr<Replica>> replicaSet;
  std::vector<std::shared_ptr<Replica>> badReplicaSet;
  std::vector<std::shared_ptr<Replica>> goodReplicaSet;
  std::shared_ptr<MetadataStore> metadataStore;
};

inline SequencerCreationResult
createSequencer(std::int32_t numberOfBadReplicas) {
  std::shared_ptr<MetadataStore>
      metadataStore = std::make_shared<InMemoryMetadataStore>();

  // Add metadata block.
  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(1);
    config.set_endindex(1000);

    metadataStore->compareAndAppendRange(0, config);
  }

  std::int32_t totalNumberOfReplicas = 5;
  std::vector<std::shared_ptr<Replica>> replicaSet;
  std::vector<std::shared_ptr<Replica>> badReplicaSet;
  std::vector<std::shared_ptr<Replica>> goodReplicaSet;

  for (std::int32_t i = 0; i < numberOfBadReplicas; i++) {
    std::shared_ptr<MockReplica> mockReplica = std::make_shared<MockReplica>();
    ON_CALL(*mockReplica, append(_, _, _))
        .WillByDefault([]() {
          return folly::makeSemiFuture<folly::Unit>
              (folly::make_exception_wrapper<NonRecoverableError>(
                  NonRecoverableError{}));
        });

    ON_CALL(*mockReplica, getLogEntry(_))
        .WillByDefault([]() {
          return folly::SemiFuture<std::variant<LogEntry, LogReadError>>
              (folly::make_exception_wrapper<NonRecoverableError>(
                  NonRecoverableError{}));
        });

    badReplicaSet.emplace_back(mockReplica);
    replicaSet.emplace_back(std::move(mockReplica));
  }

  for (std::int32_t i = 0; i < totalNumberOfReplicas - numberOfBadReplicas;
       i++) {
    std::shared_ptr<Replica>
        replica = std::make_shared<ReplicaImpl>
        ("random_name", "random_id", std::make_shared<NanoLogStoreImpl>(),
         metadataStore);

    goodReplicaSet.emplace_back(replica);
    replicaSet.emplace_back(std::move(replica));
  }

  std::shared_ptr<Sequencer>
      sequencer = std::make_shared<SequencerImpl>(replicaSet, 1);

  return {sequencer, replicaSet, badReplicaSet, goodReplicaSet, metadataStore};
}

}