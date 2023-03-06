//
// Created by Rahul  Kushwaha on 2/23/23.
//
#pragma once
#include <glog/logging.h>
#include <grpc++/grpc++.h>
#include "applications/counter/server/proto/CounterService.grpc.pb.h"

namespace rk::projects::counter_app {

class CounterAppClient {
 public:
  explicit CounterAppClient(std::shared_ptr<grpc::Channel> channel);

  std::int64_t incrementAndGet(std::int64_t incrBy);
  std::int64_t decrementAndGet(std::int64_t decrBy);
  std::int64_t getValue();

 private:
  std::unique_ptr<CounterService::Stub> stub_;
};

}
