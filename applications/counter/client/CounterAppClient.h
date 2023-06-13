//
// Created by Rahul  Kushwaha on 2/23/23.
//
#pragma once
#include "applications/counter/server/proto/CounterService.grpc.pb.h"
#include "folly/experimental/coro/Task.h"
#include <glog/logging.h>
#include <grpc++/grpc++.h>

namespace rk::projects::counter_app {

class CounterAppClient {
public:
  explicit CounterAppClient(std::shared_ptr<grpc::Channel> channel);

  folly::coro::Task<std::int64_t> incrementAndGet(std::string key,
                                                  std::int64_t incrBy);
  folly::coro::Task<std::int64_t> decrementAndGet(std::string key,
                                                  std::int64_t decrBy);
  folly::coro::Task<std::int64_t> getValue(std::string key);

private:
  std::unique_ptr<CounterService::Stub> stub_;
};

} // namespace rk::projects::counter_app
