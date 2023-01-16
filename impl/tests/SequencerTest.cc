//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <gtest/gtest.h>
#include "../SequencerImpl.h"
#include "../NanoLogStoreImpl.h"
#include "TestUtils.h"

namespace rk::project::counter {
using namespace testing;

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
  auto goodReplicaSet = sequencerCreationResult.goodReplicaSet;

  std::string logEntry{"Hello World"};
  ASSERT_THROW(sequencer->append(logEntry).get(), std::exception);

  for (auto &replica: goodReplicaSet) {
    auto result = replica->getLogEntry(1).get();
    ASSERT_TRUE(std::holds_alternative<LogEntry>(result));
    ASSERT_EQ(std::get<LogEntry>(result).logId, 1);
    ASSERT_EQ(std::get<LogEntry>(result).payload, logEntry);
  }
}

}