//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <folly/executors/InlineExecutor.h>
#include <gtest/gtest.h>
#include <mutex>
#include <random>

#include "log/impl/InMemoryMetadataStore.h"
#include "log/impl/NanoLogStoreImpl.h"
#include "log/impl/ReplicaImpl.h"
#include "log/impl/RocksReplica.h"
#include "persistence/RocksDbFactory.h"
#include "persistence/RocksKVStoreLite.h"

namespace rk::projects::durable_log {

namespace {

enum class ReplicaType {
  InMemory,
  RocksDb,
};

struct SequencerCreationResult {
  std::shared_ptr<Replica> replica;
  std::shared_ptr<MetadataStore> metadataStore;
};

} // namespace

class ReplicaTests : public testing::TestWithParam<ReplicaType> {
protected:
  std::shared_ptr<rocksdb::DB> rocks_;
  SequencerCreationResult creationResult_;
  persistence::RocksDbFactory::RocksDbConfig config_{
      .path = "/tmp/replica_tests", .createIfMissing = true};

protected:
  SequencerCreationResult createReplica(bool inMemoryReplica = true) {
    std::shared_ptr<NanoLogStore> nanoLogStore =
        std::make_shared<NanoLogStoreImpl>();
    std::shared_ptr<MetadataStore> metadataStore =
        std::make_shared<InMemoryMetadataStore>();

    // Add metadata block.
    {
      MetadataConfig config;
      config.set_version_id(1);
      config.set_previous_version_id(0);
      config.set_start_index(500);
      config.set_end_index(1000);

      metadataStore->compareAndAppendRange(0, config);
    }

    if (inMemoryReplica) {
      return {std::make_shared<ReplicaImpl>(ReplicaImpl{
                  "random_id", "random_name", nanoLogStore, metadataStore}),
              metadataStore};
    }

    rocks_ = persistence::RocksDbFactory::provideSharedPtr(config_);
    auto kvStore = std::make_shared<persistence::RocksKVStoreLite>(rocks_);
    auto rocksReplica = std::make_shared<RocksReplica>(
        "random_id", "random_name", metadataStore, kvStore);
    return {rocksReplica, metadataStore};
  }

  void SetUp() override {
    switch (GetParam()) {
    case ReplicaType::RocksDb:
      creationResult_ = createReplica(false);
      break;
    case ReplicaType::InMemory:
      creationResult_ = createReplica();
      break;
    default:
      throw std::runtime_error{"unknown replica type"};
    }
  }

  void TearDown() override {}
};

TEST_P(ReplicaTests, AppendLogEntryWhenMetadataInstalled) {
  auto replica = creationResult_.replica;
  auto metadataStore = creationResult_.metadataStore;
  auto versionId = metadataStore->getCurrentVersionId();

  // Append a log entry.
  {
    std::string logEntry{"Hello World"};
    ASSERT_NO_THROW(replica->append({}, versionId, 500, logEntry).semi().get());

    auto result = replica->getLogEntry(versionId, 500).semi().get();

    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(500, std::get<LogEntry>(result).logId);
    ASSERT_EQ(logEntry, std::get<LogEntry>(result).payload);
  }
}

TEST_P(ReplicaTests, AppendLogEntryWhenNanoLogIsSealed) {
  auto replica = creationResult_.replica;
  auto metadataStore = creationResult_.metadataStore;
  auto versionId = metadataStore->getCurrentVersionId();

  {
    ASSERT_NO_THROW(
        replica->append({}, versionId, 500, "Hello World").semi().get());
    LOG(INFO) << "append complete" << std::endl;
  }

  replica->seal(versionId);

  std::string logEntry{"Hello World"};
  ASSERT_THROW(replica->append({}, versionId, 501, logEntry).semi().get(),
               NanoLogSealedException);
}

TEST_P(ReplicaTests,
       AppendLogEntryWhenNanoLogIsSealedButOverrideFlagIsPresent) {
  auto replica = creationResult_.replica;
  auto metadataStore = creationResult_.metadataStore;
  auto versionId = metadataStore->getCurrentVersionId();

  ASSERT_NO_THROW(
      replica->append({}, versionId, 500, "Hello World").semi().get());

  replica->seal(versionId);

  std::string logEntry{"Hello World"};
  ASSERT_NO_THROW(
      replica->append({}, versionId, 501, logEntry, true).semi().get());
}

TEST_P(ReplicaTests, AppendDuplicateLogEntry) {
  auto replica = creationResult_.replica;
  auto versionId = creationResult_.metadataStore->getCurrentVersionId();

  ASSERT_NO_THROW(
      replica->append({}, versionId, 500, "Hello World").semi().get());
  ASSERT_THROW(replica->append({}, versionId, 500, "Hello World").semi().get(),
               NanoLogLogPositionAlreadyOccupied);
}

TEST_P(ReplicaTests, UnOrderedAppendAlwaysFinishInOrder) {
  auto replica = creationResult_.replica;
  auto versionId = creationResult_.metadataStore->getCurrentVersionId();
  auto startIndex = 500;

  replica
      ->append({}, versionId, startIndex,
               "Random Text" + std::to_string(startIndex))
      .semi()
      .get();

  // 501 is the missing entry.
  std::vector<std::int32_t> elements;
  std::int32_t numberOfElements = 10;
  for (std::int32_t iteration = 2; iteration <= numberOfElements; iteration++) {
    elements.push_back(500 + iteration);
  }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(elements.begin(), elements.end(), g);

  std::vector<folly::SemiFuture<LogId>> futures;
  for (auto element : elements) {
    std::cout << element << " " << std::endl;
    auto future = replica
                      ->append({}, versionId, element,
                               "Random Text" + std::to_string(element))
                      .semi()
                      .via(&folly::InlineExecutor::instance())
                      .thenValue([element](auto &&r) { return element; })
                      .thenError([](auto &&e) -> folly::Try<int> { throw e; });

    futures.emplace_back(std::move(future));
  }
  std::cout << std::endl;

  for (auto &future : futures) {
    ASSERT_FALSE(future.isReady());
  }

  // Append the missing entry, 501
  replica->append({}, versionId, 501, "Random Text" + std::to_string(500))
      .semi()
      .get();
  folly::collectAll(futures.begin(), futures.end()).get();

  for (auto &future : futures) {
    ASSERT_NO_THROW(future.value());
  }
}

INSTANTIATE_TEST_SUITE_P(ReplicaParameterizedTests, ReplicaTests,
                         testing::Values(ReplicaType::InMemory));

} // namespace rk::projects::durable_log