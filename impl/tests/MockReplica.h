//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <gmock/gmock.h>
#include <variant>

#include "../../include/Replica.h"

namespace rk::project::counter {

class MockReplica: public Replica {
 public:
  MOCK_METHOD(std::string, getId, (), (override));
  MOCK_METHOD(std::string, getName, (), (override));

  MOCK_METHOD(folly::SemiFuture<folly::Unit>,
              append,
              (LogId logId, std::string logEntryPayload, bool skipSeal),
              (override));

  MOCK_METHOD((folly::SemiFuture<std::variant<LogEntry, LogReadError>>),
              getLogEntry,
              (LogId logId), (override));

  MOCK_METHOD(LogId, getLocalCommitIndex, (), (override));

  MOCK_METHOD(LogId, seal, (VersionId versionId), (override));
};

}