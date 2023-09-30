//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include "../NanoLogStoreImpl.h"
#include "../SequencerImpl.h"
#include "TestUtils.h"

#include <gtest/gtest.h>

namespace rk::projects::durable_log {
using namespace testing;

TEST(SequencerTest, appendLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  auto sequencer = sequencerCreationResult.sequencer;
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto versionId =
      sequencerCreationResult.metadataStore->getCurrentVersionId().semi().get();

  std::string logEntry{"Hello World"};
  ASSERT_EQ(sequencer->append(logEntry).get(), 1);

  for (auto& replica : replicaSet) {
    auto result = replica->getLogEntry(versionId, 1).semi().get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(std::get<LogEntry>(result).logId, 1);
    ASSERT_EQ(std::get<LogEntry>(result).payload, logEntry);
  }
}

TEST(SequencerTest, appendLogEntryWithMajorityFailing) {
  auto sequencerCreationResult = createSequencer(3);
  auto sequencer = sequencerCreationResult.sequencer;
  auto replicaSet = sequencerCreationResult.replicaSet;
  auto goodReplicaSet = sequencerCreationResult.goodReplicaSet;
  auto versionId =
      sequencerCreationResult.metadataStore->getCurrentVersionId().semi().get();

  std::string logEntry{"Hello World"};
  ASSERT_THROW(sequencer->append(logEntry).get(), std::exception);

  for (auto& replica : goodReplicaSet) {
    auto result = replica->getLogEntry(versionId, 1).semi().get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(std::get<LogEntry>(result).logId, 1);
    ASSERT_EQ(std::get<LogEntry>(result).payload, logEntry);
  }
}

}  // namespace rk::projects::durable_log