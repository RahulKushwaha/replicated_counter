//
// Created by Rahul  Kushwaha on 1/16/23.
//

#include <gtest/gtest.h>
#include "TestUtils.h"
#include "../../include/VirtualLog.h"
#include "../VirtualLogImpl.h"
#include "../../utils/FutureUtils.h"

namespace rk::project::counter {

TEST(VirtualLogTests, AppendOneLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>("virtual_log_id",
                                             "virtual_log_name",
                                             sequencerCreationResult.sequencer,
                                             sequencerCreationResult.replicaSet,
                                             sequencerCreationResult.metadataStore);

  ASSERT_EQ(log->append("hello_world").get(), 1);
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
      log = std::make_shared<VirtualLogImpl>("virtual_log_id",
                                             "virtual_log_name",
                                             sequencerCreationResult.sequencer,
                                             sequencerCreationResult.replicaSet,
                                             sequencerCreationResult.metadataStore);

  ASSERT_NO_THROW(log->reconfigure());

  // Successfully installed a new metadata block.
  auto versionId = metadataStore->getCurrentVersionId();
  ASSERT_EQ(versionId, 2);

  auto config = metadataStore->getConfig(1);
  ASSERT_TRUE(config.has_value());


  LOG(INFO) << "Last Entry: " << config->endindex();
  for (auto logId = config->startindex(); logId < config->endindex(); logId++) {
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

}