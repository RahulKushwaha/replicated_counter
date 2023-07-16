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
  co_return app_->apply(t);
}

} // namespace rk::projects::counter_app