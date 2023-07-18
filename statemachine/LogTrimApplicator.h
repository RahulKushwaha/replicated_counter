//
// Created by Rahul  Kushwaha on 7/16/23.
//
#pragma once
#include "folly/executors/ThreadPoolExecutor.h"
#include "folly/experimental/coro/AsyncScope.h"
#include "log/proto/LogEntry.pb.h"
#include "persistence/KVStoreLite.h"
#include "statemachine/include/StateMachine.h"

namespace rk::projects::state_machine {

template <typename R>
class LogTrimApplicator : public Applicator<durable_log::LogEntry_1, R> {
public:
  explicit LogTrimApplicator(std::shared_ptr<persistence::KVStoreLite> kvStore)
      : kvStore_{std::move(kvStore)}, asyncScope_{}, executor_{nullptr} {}

  coro<R> apply(durable_log::LogEntry_1 t) override {
    if (t.has_log_trim_entry()) {
      auto &logTrimEntry = t.log_trim_entry();
      auto result = co_await kvStore_->get(std::string{KEY});
      durable_log::LogTrimView logTrimView{};
      if (result.has_value()) {
        auto parseResult = logTrimView.ParseFromString(result.value());
        if (!parseResult) {
          throw std::runtime_error{"trim entry parse error"};
        }
      }

      (*logTrimView.mutable_entries())[logTrimEntry.replica_id()] =
          logTrimEntry;

      logTrimView.mutable_entries()->emplace(logTrimEntry.replica_id(),
                                             logTrimEntry);

      asyncScope_.add(trimLogs().scheduleOn(executor_.get()));
    }

    co_return folly::unit;
  }

private:
  coro<void> trimLogs() { co_return; }

private:
  std::shared_ptr<persistence::KVStoreLite> kvStore_;
  folly::coro::AsyncScope asyncScope_;
  std::shared_ptr<folly::ThreadPoolExecutor> executor_;

  static constexpr std::string_view KEY{"LOG_TRIM_VIEW"};
};

} // namespace rk::projects::state_machine
