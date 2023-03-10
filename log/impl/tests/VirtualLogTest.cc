//
// Created by Rahul  Kushwaha on 1/16/23.
//

#include <gtest/gtest.h>
#include "TestUtils.h"
#include "../../include/VirtualLog.h"
#include "../VirtualLogImpl.h"
#include "../../utils/FutureUtils.h"

namespace rk::projects::durable_log {

TEST(VirtualLogTests, AppendOneLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  ASSERT_EQ(log->append("hello_world").get(), 1);
}

TEST(VirtualLogTests, GetOneLogEntry) {
  std::string logEntryPayload{"hello_world"};
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

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
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  // Fetch the appended log entry.
  ASSERT_THROW(log->getLogEntry(1).get(), std::exception);
}

TEST(VirtualLogTests, GetOneLogEntryWithMinorityFailing) {
  std::string logEntryPayload{"hello_world"};
  auto sequencerCreationResult = createSequencer(2);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  ASSERT_EQ(log->append(logEntryPayload).get(), 1);

  // Fetch the appended log entry.
  auto logEntryResult = log->getLogEntry(1).get();
  ASSERT_TRUE(std::holds_alternative<LogEntry>(logEntryResult));

  ASSERT_EQ(logEntryPayload, std::get<LogEntry>(logEntryResult).payload);
  ASSERT_EQ(1, std::get<LogEntry>(logEntryResult).logId);
}

TEST(VirtualLogTests, ReconfigureOnEmptySegment) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  ASSERT_NO_THROW(log->reconfigure());
}

TEST(VirtualLogTests, ReconfigureOnEmptySegmentMultipleTimes) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  for (int i = 0; i < 10; i++) {
    ASSERT_NO_THROW(log->reconfigure());
  }
}


TEST(VirtualLogTests, Reconfigure) {
  auto sequencerCreationResult = createSequencer(0);
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto metadataStore = sequencerCreationResult.metadataStore;

  int limit = 10;
  for (auto &replica: replicaSet) {
    for (int i = 1; i <= limit; i++) {
      replica->append(i, "Log_Entry" + std::to_string(i)).get();
    }

    limit += 10;
  }

  int i = 10;
  for (auto &replica: replicaSet) {
    ASSERT_EQ(replica->getLocalCommitIndex(), i + 1);
    i += 10;
  }

  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  ASSERT_NO_THROW(log->reconfigure());

  // Successfully installed a new metadata block.
  auto versionId = metadataStore->getCurrentVersionId();
  ASSERT_EQ(versionId, 2);

  auto config = metadataStore->getConfig(1);
  ASSERT_TRUE(config.has_value());


  LOG(INFO) << "Last Entry: " << config->end_index();
  for (auto logId = config->start_index(); logId < config->end_index();
       logId++) {
    std::vector<folly::SemiFuture<LogEntry>> futures;
    for (auto &replica: replicaSet) {
      auto future = replica->getLogEntry(logId)
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

  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>
      ("virtual_log_id",
       "virtual_log_name",
       sequencerCreationResult.sequencer,
       sequencerCreationResult.replicaSet,
       sequencerCreationResult.metadataStore,
       sequencerCreationResult.initialMetadataConfig.version_id());

  constexpr int limit = 100;
  int totalNumberOfReconfigurations = 0;
  for (int i = 1; i <= limit; i++) {
    auto logId = log->append("Log_Entry" + std::to_string(i)).get();
    ASSERT_EQ(logId, i);

    if (i % 2 == 0) {
      log->reconfigure();
      totalNumberOfReconfigurations++;

      if (i % 6 == 0 || i % 8 == 0) {
        log->reconfigure();
        totalNumberOfReconfigurations++;
      }
    }
  }

  // Successfully installed a new metadata block.
  auto versionId = metadataStore->getCurrentVersionId();
  auto finalVersionId = totalNumberOfReconfigurations + 1;
  ASSERT_EQ(versionId, finalVersionId);

  for (auto logId = 1; logId <= limit; logId++) {
    auto logPayload = log->getLogEntry(logId).get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(logPayload));
  }
}

}