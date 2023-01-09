//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <gtest/gtest.h>
#include "../SequencerImpl.h"
#include "../ReplicaImpl.h"
#include "../InMemoryMetadataStore.h"
#include "../NanoLogStoreImpl.h"
#include "MockReplica.h"

namespace rk::project::counter {
using namespace testing;

namespace {
struct CreationResult {
  std::shared_ptr<Sequencer> sequencer;
  std::vector<std::shared_ptr<Replica>> replicaSet;
};

CreationResult createSequencer(std::int32_t numberOfBadReplicas) {
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

  for (std::int32_t i = 0; i < numberOfBadReplicas; i++) {
    std::shared_ptr<MockReplica> mockReplica = std::make_shared<MockReplica>();
    ON_CALL(*mockReplica, append(_, _))
        .WillByDefault([]() {
          return folly::makeSemiFuture<folly::Unit>
              (folly::make_exception_wrapper<std::exception>(
                  std::exception{}));
        });

    replicaSet.emplace_back(std::move(mockReplica));
  }

  for (std::int32_t i = 0; i < totalNumberOfReplicas - numberOfBadReplicas;
       i++) {
    std::shared_ptr<Replica>
        replica = std::make_shared<ReplicaImpl>
        ("random_name", "random_id", std::make_shared<NanoLogStoreImpl>(),
         metadataStore);

    replicaSet.emplace_back(std::move(replica));
  }

  std::shared_ptr<Sequencer>
      sequencer = std::make_shared<SequencerImpl>(replicaSet, 1);

  return {sequencer, replicaSet};
}

}

TEST(SequencerTest, appendLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  auto sequencer = sequencerCreationResult.sequencer;
  auto replicaSet = sequencerCreationResult.replicaSet;

  std::string logEntry{"Hello World"};
  ASSERT_EQ(sequencer->append(logEntry).get(), 1);

  for (auto &replica: replicaSet) {
    auto result = replica->getLogEntry(1).get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(std::get<LogEntry>(result).logId, 1);
    ASSERT_EQ(std::get<LogEntry>(result).payload, logEntry);
  }
}

TEST(SequencerTest, appendLogEntryWithMajorityFailing) {
  auto sequencerCreationResult = createSequencer(3);
  auto sequencer = sequencerCreationResult.sequencer;
  auto replicaSet = sequencerCreationResult.replicaSet;

  std::string logEntry{"Hello World"};
  ASSERT_THROW(sequencer->append(logEntry).get(), std::exception);
}

}