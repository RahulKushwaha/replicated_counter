//
// Created by Rahul  Kushwaha on 1/16/23.
//

#include "log/include/VirtualLog.h"
#include "TestUtils.h"
#include "log/impl/VirtualLogImpl.h"
#include "log/utils/FutureUtils.h"
#include <gtest/gtest.h>

namespace rk::projects::durable_log {

TEST(VirtualLogTests, AppendOneLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_EQ(log->append("hello_world").get(), 1);
}

TEST(VirtualLogTests, GetOneLogEntry) {
  std::string logEntryPayload{"hello_world"};
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_EQ(log->append(logEntryPayload).get(), 1);

  // Fetch the appended log entry.
  auto logEntryResult = log->getLogEntry(1).get();
  ASSERT_TRUE(std::holds_alternative<LogEntry>(logEntryResult));

  ASSERT_EQ(logEntryPayload, std::get<LogEntry>(logEntryResult).payload);
  ASSERT_EQ(1, std::get<LogEntry>(logEntryResult).logId);
}

TEST(VirtualLogTests, GetNonExistentLogEntry) {
  std::string logEntryPayload{"hello_world"};
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  // Fetch the appended log entry.
  ASSERT_THROW(log->getLogEntry(1).get(), std::exception);
}

TEST(VirtualLogTests, GetOneLogEntryWithMinorityFailing) {
  std::string logEntryPayload{"hello_world"};
  auto sequencerCreationResult = createSequencer(2);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_EQ(log->append(logEntryPayload).get(), 1);

  // Fetch the appended log entry.
  auto logEntryResult = log->getLogEntry(1).get();
  ASSERT_TRUE(std::holds_alternative<LogEntry>(logEntryResult));

  ASSERT_EQ(logEntryPayload, std::get<LogEntry>(logEntryResult).payload);
  ASSERT_EQ(1, std::get<LogEntry>(logEntryResult).logId);
}

TEST(VirtualLogTests, ReconfigureOnEmptySegment) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_NO_THROW(
      log->reconfigure(sequencerCreationResult.initialMetadataConfig)
          .semi()
          .get());
}

TEST(VirtualLogTests, ReconfigureOnEmptySegmentMultipleTimes) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  for (int i = 0; i < 10; i++) {
    ASSERT_NO_THROW(
        log->reconfigure(sequencerCreationResult.initialMetadataConfig)
            .semi()
            .get());
  }
}

TEST(VirtualLogTests, Reconfigure) {
  auto sequencerCreationResult = createSequencer(0);
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto metadataStore = sequencerCreationResult.metadataStore;
  auto currentVersionId = metadataStore->getCurrentVersionId().semi().get();

  int limit = 10;
  for (auto &replica : replicaSet) {
    for (int i = 1; i <= limit; i++) {
      replica->append({}, currentVersionId, i, "Log_Entry" + std::to_string(i))
          .semi()
          .get();
    }

    limit += 10;
  }

  int i = 10;
  for (auto &replica : replicaSet) {
    ASSERT_EQ(replica->getCommitIndex(currentVersionId).semi().get(), i + 1);
    i += 10;
  }

  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_NO_THROW(
      log->reconfigure(sequencerCreationResult.initialMetadataConfig)
          .semi()
          .get());

  // Successfully installed a new metadata block.
  auto versionId = metadataStore->getCurrentVersionId().semi().get();
  ASSERT_EQ(versionId, 2);

  auto config = metadataStore->getConfig(1).semi().get();
  ASSERT_TRUE(config.has_value());

  LOG(INFO) << "Last Entry: " << config->end_index();
  for (auto logId = config->start_index(); logId < config->end_index();
       logId++) {
    std::vector<folly::SemiFuture<LogEntry>> futures;
    for (auto &replica : replicaSet) {
      auto future =
          replica->getLogEntry(config->version_id(), logId)
              .semi()
              .via(&folly::InlineExecutor::instance())
              .thenValue([](std::variant<LogEntry, LogReadError> &&value) {
                if (std::holds_alternative<LogEntry>(value)) {
                  return std::get<LogEntry>(value);
                }

                throw std::exception{};
              });

      futures.emplace_back(std::move(future));
    }

    ASSERT_NO_THROW(utils::anyNSuccessful(std::move(futures), 3).get());
  }
}

TEST(VirtualLogTests, ReconfigureOnSegmentMultipleTimes) {
  auto sequencerCreationResult = createSequencer();
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto metadataStore = sequencerCreationResult.metadataStore;

  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  int limit = 100;
  int totalNumberOfReconfigurations = 0;
  for (int i = 1; i <= limit; i++) {
    std::cout << std::endl;
    auto logId = log->append("Log_Entry" + std::to_string(i)).get();
    ASSERT_EQ(logId, i);

    if (i % 2 == 0) {
      log->reconfigure(sequencerCreationResult.initialMetadataConfig)
          .semi()
          .get();
      totalNumberOfReconfigurations++;

      if (i % 6 == 0 || i % 8 == 0) {
        log->reconfigure(sequencerCreationResult.initialMetadataConfig)
            .semi()
            .get();
        totalNumberOfReconfigurations++;
      }
    }
  }

  // Successfully installed a new metadata block.
  auto versionId = metadataStore->getCurrentVersionId().semi().get();
  auto finalVersionId = totalNumberOfReconfigurations + 1;
  ASSERT_EQ(versionId, finalVersionId);

  for (auto logId = 1; logId <= limit; logId++) {
    auto logPayload = log->getLogEntry(logId).get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(logPayload));
  }
}

TEST(VirtualLogTests, MajorityReplicaWithHoles) {
  auto sequencerCreationResult = createSequencer(0, 11);
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto metadataStore = sequencerCreationResult.metadataStore;
  auto sequencer = sequencerCreationResult.sequencer;
  auto versionId = metadataStore->getCurrentVersionId().semi().get();

  std::vector<std::vector<int>> logEntriesOrder{
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
      {1, 2, 3, 4, /*MISSING*/ 6, 7, 8, 9, 10},
      {1, 2, 3, 4, 5, /*MISSING*/ 7, 8, 9, 10},
      {1, 2, 3, 4, 5, 6, /*MISSING*/ 8, 9, 10},
  };

  int logEntriesIndex = 0;
  for (auto &replica : replicaSet) {
    auto &logEntries = logEntriesOrder[logEntriesIndex];
    for (const auto logEntry : logEntries) {
      replica
          ->append({}, versionId, logEntry,
                   "Log_Entry" + std::to_string(logEntry))
          .semi()
          .get();
    }

    logEntriesIndex++;
  }

  ASSERT_EQ(replicaSet[0]->getCommitIndex(versionId).semi().get(), 11);
  ASSERT_EQ(replicaSet[1]->getCommitIndex(versionId).semi().get(), 11);
  // Replica 2,3,4 have holes. Therefore, they cannot ack to the entries after
  // the holes.
  ASSERT_EQ(replicaSet[2]->getCommitIndex(versionId).semi().get(), 5);
  ASSERT_EQ(replicaSet[3]->getCommitIndex(versionId).semi().get(), 6);
  ASSERT_EQ(replicaSet[4]->getCommitIndex(versionId).semi().get(), 7);

  std::shared_ptr<VirtualLog> log = std::make_shared<VirtualLogImpl>(
      "virtual_log_id", "virtual_log_name", sequencerCreationResult.sequencer,
      sequencerCreationResult.replicaSet, sequencerCreationResult.metadataStore,
      sequencerCreationResult.initialMetadataConfig.version_id(),
      sequencerCreationResult.registry);

  ASSERT_NO_THROW(
      log->append("Hello World").get(std::chrono::milliseconds(1000)));
}
} // namespace rk::projects::durable_log
