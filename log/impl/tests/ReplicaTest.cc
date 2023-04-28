//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <gtest/gtest.h>
#include <random>
#include <mutex>
#include <folly/executors/InlineExecutor.h>

#include "../ReplicaImpl.h"
#include "../NanoLogStoreImpl.h"
#include "../InMemoryMetadataStore.h"

namespace rk::projects::durable_log {

namespace {
struct SequencerCreationResult {
  ReplicaImpl replica;
  std::shared_ptr<NanoLogStore> nanoLogStore;
  std::shared_ptr<MetadataStore> metadataStore;
};

SequencerCreationResult createReplica() {
  std::shared_ptr<NanoLogStore>
      nanoLogStore = std::make_shared<NanoLogStoreImpl>();
  std::shared_ptr<MetadataStore>
      metadataStore = std::make_shared<InMemoryMetadataStore>();

  // Add metadata block.
  {
    MetadataConfig config;
    config.set_version_id(1);
    config.set_previous_version_id(0);
    config.set_start_index(500);
    config.set_end_index(1000);

    metadataStore->compareAndAppendRange(0, config);
  }

  return {ReplicaImpl{"random_id", "random_name", nanoLogStore, metadataStore},
          nanoLogStore, metadataStore};
}

}

TEST(ReplicaTest, AppendLogEntryWhenMetadataInstalled) {
  SequencerCreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto metadataStore = creationResult.metadataStore;
  auto versionId = metadataStore->getCurrentVersionId();

  // Append a log entry.
  {
    std::string logEntry{"Hello World"};
    ASSERT_NO_THROW(replica.append(versionId, 500, logEntry).get());

    auto result = replica.getLogEntry(versionId, 500).get();

    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(500, std::get<LogEntry>(result).logId);
    ASSERT_EQ(logEntry, std::get<LogEntry>(result).payload);
  }
}

TEST(ReplicaTest, AppendLogEntryWhenNanoLogIsSealed) {
  SequencerCreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto metadataStore = creationResult.metadataStore;
  auto nanoLogStore = creationResult.nanoLogStore;
  auto versionId = metadataStore->getCurrentVersionId();

  ASSERT_NO_THROW(replica.append(versionId, 500, "Hello World").get());

  auto nanoLog = nanoLogStore->getNanoLog(1);
  nanoLog->seal();

  std::string logEntry{"Hello World"};
  ASSERT_THROW(replica.append(versionId, 501, logEntry).get(),
               NanoLogSealedException);
}

TEST(ReplicaTest, AppendLogEntryWhenNanoLogIsSealedButOverrideFlagIsPresent) {
  SequencerCreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto metadataStore = creationResult.metadataStore;
  auto nanoLogStore = creationResult.nanoLogStore;
  auto versionId = metadataStore->getCurrentVersionId();

  ASSERT_NO_THROW(replica.append(versionId, 500, "Hello World").get());

  auto nanoLog = nanoLogStore->getNanoLog(1);
  nanoLog->seal();

  std::string logEntry{"Hello World"};
  ASSERT_NO_THROW(replica.append(versionId, 501, logEntry, true).get());
}

TEST(ReplicaTest, AppendDuplicateLogEntry) {
  SequencerCreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto versionId = creationResult.metadataStore->getCurrentVersionId();

  ASSERT_NO_THROW(replica.append(versionId, 500, "Hello World").get());
  ASSERT_THROW(replica.append(versionId, 500, "Hello World").get(),
               NanoLogLogPositionAlreadyOccupied);
}

TEST(ReplicaTest, UnOrderedAppendAlwaysFinishInOrder) {
  SequencerCreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto versionId = creationResult.metadataStore->getCurrentVersionId();

  std::vector<std::int32_t> elements;
  std::int32_t numberOfElements = 10;
  for (std::int32_t iteration = 1; iteration <= numberOfElements; iteration++) {
    elements.push_back(500 + iteration);
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(elements.begin(), elements.end(), g);

  std::vector<folly::SemiFuture<folly::Unit>> futures;
  for (auto element: elements) {
    auto future =
        replica.append(versionId,
                       element,
                       "Random Text" + std::to_string(element));

    futures.emplace_back(std::move(future));
  }

  for (auto &future: futures) {
    ASSERT_FALSE(future.poll().has_value());
  }

  replica.append(versionId, 500, "Random Text" + std::to_string(500)).get();
  folly::collectAll(futures.begin(), futures.end()).get();

  for (auto &future: futures) {
    ASSERT_NO_THROW(future.value());
  }
}

}