//
// Created by Rahul  Kushwaha on 7/15/23.
//
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/CounterApp.h"

namespace rk::projects::counter_app {

CounterApplicator::CounterApplicator(std::shared_ptr<CounterApp> app)
    : app_{std::move(app)} {}

folly::coro::Task<applicatorOutput_t>
CounterApplicator::apply(applicatorInput_t t) {
  if (t.has_single_log_entry()) {
    CounterLogEntries entries{};
    auto parseResult = entries.ParseFromString(t.single_log_entry().payload());
    if (!parseResult) {
      throw std::runtime_error{"log entry could not be parsed"};
    }

    LOG_EVERY_T(INFO, 0.01) << "Applying Log Entry";
    co_return app_->apply(entries, t.single_log_entry().id());
  }

  throw std::runtime_error{"unknown type of entry to apply"};
}

} // namespace rk::projects::counter_app