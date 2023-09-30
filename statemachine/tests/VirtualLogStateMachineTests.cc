//
// Created by Rahul  Kushwaha on 9/9/23.
//
#include "applications/counter/test/InMemoryFakeVirtualLog.h"
#include "folly/experimental/coro/Collect.h"
#include "statemachine/SinkStateMachine.h"
#include "statemachine/VirtualLogStateMachine.h"

#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

namespace rk::projects::state_machine {
using namespace rk::projects::counter_app;

namespace {

struct StmStack {
  std::shared_ptr<VirtualLogStateMachine<LogEntry_1>> virtualLogStm;
  std::shared_ptr<SinkStateMachine> topStm;
};

std::vector<StmStack> getStateMachineStacks(std::int32_t numberOfStacks = 1) {
  auto log = std::make_shared<InMemoryFakeVirtualLog>();

  std::vector<StmStack> stacks;
  for (int i = 0; i < numberOfStacks; i++) {
    auto logStm = std::make_shared<VirtualLogStateMachine<LogEntry_1>>(log);
    auto topStm = std::make_shared<SinkStateMachine>(logStm);
    logStm->setUpstreamStateMachine(topStm);
    stacks.emplace_back(StmStack{.virtualLogStm = std::move(logStm),
                                 .topStm = std::move(topStm)});
  }

  return stacks;
}

bool areAllLogEntriesSame(std::vector<std::vector<LogEntry_1>> entries) {
  std::vector<LogEntry_1>& prevEntries = entries.at(0);

  for (std::size_t index = 1; index < entries.size(); index++) {
    auto logEntries = entries.at(index);

    if (prevEntries.size() != logEntries.size()) {
      return false;
    }

    for (std::size_t logIndex = 0; logIndex < logEntries.size(); logIndex++) {
      if (!google::protobuf::util::MessageDifferencer::Equals(
              prevEntries[logIndex], logEntries[logIndex])) {
        return false;
      }
    }
  }

  return true;
}

}  // namespace

TEST(VirtualLogStateMachineTests, singleLogEntryWithSingleStm) {
  auto stmStacks = getStateMachineStacks();
  const auto& topStm = stmStacks.at(0).topStm;

  SingleLogEntry singleLogEntry{};
  singleLogEntry.set_id(1);
  singleLogEntry.set_payload("hello world");

  LogEntry_1 logEntry{};
  logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
  auto result = topStm->append(std::move(logEntry)).semi().get();

  ASSERT_EQ(result.single_log_entry().id(), 1);
}

TEST(VirtualLogStateMachineTests, multipleLogEntriesWithSingleStm) {
  auto stmStacks = getStateMachineStacks();
  const auto& topStm = stmStacks.at(0).topStm;

  for (int j = 0, id = 1; j < 10; j++) {

    std::vector<folly::coro::Task<LogEntry_1>> tasks;
    for (int i = 0; i < 10; i++) {
      SingleLogEntry singleLogEntry{};
      singleLogEntry.set_id(id++);
      singleLogEntry.set_payload("hello world");

      LogEntry_1 logEntry{};
      logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
      auto result = topStm->append(std::move(logEntry));
      tasks.emplace_back(std::move(result));
    }

    folly::coro::collectAllRange(std::move(tasks)).semi().get();
  }
}

TEST(VirtualLogStateMachineTests, callSyncMultipleTimes) {
  auto stmStacks = getStateMachineStacks();
  const auto& topStm = stmStacks.at(0).topStm;

  for (int j = 0, id = 1; j < 10; j++) {

    std::vector<folly::coro::Task<LogEntry_1>> tasks;
    for (int i = 0; i < 10; i++) {
      SingleLogEntry singleLogEntry{};
      singleLogEntry.set_id(id++);
      singleLogEntry.set_payload("hello world");

      LogEntry_1 logEntry{};
      logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
      auto result = topStm->append(std::move(logEntry));
      tasks.emplace_back(std::move(result));
    }

    folly::coro::collectAllRange(std::move(tasks)).semi().get();
  }

  for (int i = 0; i < 100; i++) {
    ASSERT_NO_THROW(topStm->sync().semi().get());
  }
}

TEST(VirtualLogStateMachineTests, multipleLogEntryInEachStmSequentially) {
  auto stmStacks = getStateMachineStacks(5);
  LogId id{1};
  for (const auto& stmStack : stmStacks) {
    const auto& stm = stmStack.topStm;
    for (int i = 0; i < 10; i++) {
      SingleLogEntry singleLogEntry{};
      singleLogEntry.set_id(id++);
      singleLogEntry.set_payload("hello world");

      LogEntry_1 logEntry{};
      logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
      ASSERT_NO_THROW(stm->append(std::move(logEntry)).semi().get());
    }

    auto logs = stm->getAllLogEntries();
    ASSERT_TRUE(std::is_sorted(
        logs.begin(), logs.end(), [](const LogEntry_1& x, const LogEntry_1& y) {
          return x.single_log_entry().id() < y.single_log_entry().id();
        }));
  }
}

TEST(VirtualLogStateMachineTests, multipleLogEntryInEachStmParallely) {
  auto stmStacks = getStateMachineStacks(5);

  folly::CPUThreadPoolExecutor threadPoolExecutor{10};
  std::vector<folly::SemiFuture<folly::Unit>> tasks;
  std::atomic_int64_t errors{0};
  std::atomic_int64_t id{1};
  for (const auto& stmStack : stmStacks) {
    const auto& stm = stmStack.topStm;
    auto task =
        folly::coro::co_invoke(
            [stm, &id, &errors]() -> folly::coro::Task<void> {
              for (int i = 0; i < 10; i++) {
                SingleLogEntry singleLogEntry{};
                singleLogEntry.set_id(id++);
                singleLogEntry.set_payload("hello world");

                LogEntry_1 logEntry{};
                logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
                try {
                  stm->append(std::move(logEntry)).semi().get();
                } catch (...) {
                  errors++;
                }
              }

              co_return;
            })
            .semi()
            .via(&threadPoolExecutor)
            .semi();

    tasks.emplace_back(std::move(task));
  }

  folly::collectAll(tasks).get();

  for (auto& stmStack : stmStacks) {
    ASSERT_NO_THROW(stmStack.topStm->sync().semi().get());
  }

  std::vector<std::vector<LogEntry_1>> entries;
  for (const auto& stm : stmStacks) {
    entries.emplace_back(stm.topStm->getAllLogEntries());
  }

  ASSERT_TRUE(areAllLogEntriesSame(entries));
}

TEST(VirtualLogStateMachineTests, addToFirstStmButSyncMultiple) {
  auto stmStacks = getStateMachineStacks(5);

  LogId id{1};
  auto& firstStm = stmStacks.at(0).topStm;
  for (int i = 0; i < 10; i++) {
    SingleLogEntry singleLogEntry{};
    singleLogEntry.set_id(id++);
    singleLogEntry.set_payload("hello world");

    LogEntry_1 logEntry{};
    logEntry.mutable_single_log_entry()->CopyFrom(singleLogEntry);
    ASSERT_NO_THROW(firstStm->append(std::move(logEntry)).semi().get());
  }

  for (auto& stmStack : stmStacks) {
    ASSERT_NO_THROW(stmStack.topStm->sync().semi().get());
  }

  std::vector<std::vector<LogEntry_1>> entries;
  for (const auto& stm : stmStacks) {
    entries.emplace_back(stm.topStm->getAllLogEntries());
  }

  ASSERT_TRUE(areAllLogEntriesSame(entries));
}

}  // namespace rk::projects::state_machine