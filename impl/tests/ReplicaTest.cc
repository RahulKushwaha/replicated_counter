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

namespace rk::project::counter {

namespace {
struct CreationResult {
  ReplicaImpl replica;
  std::shared_ptr<NanoLogStore> nanoLogStore;
  std::shared_ptr<MetadataStore> metadataStore;
};

CreationResult createReplica() {
  std::shared_ptr<NanoLogStore>
      nanoLogStore = std::make_shared<NanoLogStoreImpl>();
  std::shared_ptr<MetadataStore>
      metadataStore = std::make_shared<InMemoryMetadataStore>();

  // Add metadata block.
  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(500);
    config.set_endindex(1000);

    metadataStore->compareAndAppendRange(0, config);
  }

  return {ReplicaImpl{"random_id", "random_name", nanoLogStore, metadataStore},
          nanoLogStore, metadataStore};
}

}

TEST(ReplicaTest, AppendLogEntryIfMetadataNotInstalled) {
  CreationResult result = createReplica();
  ASSERT_THROW(result.replica.append(1, std::string{"Hello World!"}).get(),
               MetadataBlockNotPresent);
}

TEST(ReplicaTest, AppendLogEntryWhenMetadataInstalled) {
  CreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto metadataStore = creationResult.metadataStore;

  // Append a log entry.
  {
    std::string logEntry{"Hello World"};
    ASSERT_NO_THROW(replica.append(500, logEntry).get());

    auto result = replica.getLogEntry(500).get();

    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(500, std::get<LogEntry>(result).logId);
    ASSERT_EQ(logEntry, std::get<LogEntry>(result).payload);
  }
}

TEST(ReplicaTest, AppendLogEntryWhenNanoLogIsSealed) {
  CreationResult creationResult = createReplica();
  auto replica = creationResult.replica;
  auto metadataStore = creationResult.metadataStore;
  auto nanoLogStore = creationResult.nanoLogStore;

  ASSERT_NO_THROW(replica.append(500, "Hello World").get());

  auto nanoLog = nanoLogStore->getNanoLog(1);
  nanoLog->seal();

  std::string logEntry{"Hello World"};
  ASSERT_THROW(replica.append(501, logEntry).get(), NanoLogSealedException);
}

TEST(ReplicaTest, AppendDuplicateLogEntry) {
  CreationResult creationResult = createReplica();
  auto replica = creationResult.replica;

  ASSERT_NO_THROW(replica.append(500, "Hello World").get());
  ASSERT_THROW(replica.append(500, "Hello World").get(),
               NanoLogLogPositionAlreadyOccupied);
}

TEST(ReplicaTest, UnOrderedAppendAlwaysFinishInOrder) {
  folly::InlineExecutor inlineExecutor{};
  CreationResult creationResult = createReplica();
  auto replica = creationResult.replica;

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
        replica.append(element, "Random Text" + std::to_string(element));

    futures.emplace_back(std::move(future));
  }

  for (auto &future: futures) {
    ASSERT_FALSE(future.poll().has_value());
  }

  replica.append(500, "Random Text" + std::to_string(500)).get();
  folly::collectAll(futures.begin(), futures.end()).get();

  for (auto &future: futures) {
    ASSERT_TRUE(future.hasValue());
  }
}

}