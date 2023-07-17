//
// Created by Rahul  Kushwaha on 1/8/23.
//
#include <gmock/gmock.h>
#include <variant>

#include "log/include/Replica.h"

namespace rk::projects::durable_log {

class MockReplica : public Replica {
public:
  MOCK_METHOD(std::string, getId, (), (override));
  MOCK_METHOD(std::string, getName, (), (override));

  MOCK_METHOD(coro<folly::Unit>, append,
              (std::optional<LogId> globalCommitIndex, VersionId versionId,
               LogId logId, std::string logEntryPayload, bool skipSeal),
              (override));

  MOCK_METHOD((coro<std::variant<LogEntry, LogReadError>>), getLogEntry,
              (VersionId versionId, LogId logId), (override));

  MOCK_METHOD(coro<LogId>, getCommitIndex, (VersionId versionId), (override));

  MOCK_METHOD(coro<LogId>, seal, (VersionId versionId), (override));

  MOCK_METHOD(coro<LogId>, trim, (VersionId versionId, LogId logId),
              (override));
};

} // namespace rk::projects::durable_log