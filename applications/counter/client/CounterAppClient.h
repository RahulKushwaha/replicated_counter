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
  explicit CounterAppClient(std::shared_ptr<grpc::Channel> channel) :
      stub_{CounterService::NewStub(std::move(channel))} {}

  std::int64_t incrementAndGet(std::int64_t incrBy) {
    IncrementRequest request{};
    request.set_incr_by(incrBy);

    CounterValue response;
    grpc::ClientContext context;
    grpc::Status status;
    stub_->IncrementAndGet(&context, request, &response);

    if (!status.ok()) {
      LOG(INFO) << "Failed: " << status.error_details();
    }

    return response.value();
  }

  std::int64_t decrementAndGet(std::int64_t decrBy) {
    DecrementRequest request{};
    request.set_decr_by(decrBy);

    CounterValue response;
    grpc::ClientContext context;
    grpc::Status status;
    stub_->DecrementAndGet(&context, request, &response);

    if (!status.ok()) {
      LOG(INFO) << "Failed: " << status.error_details();
    }

    return response.value();
  }

  std::int64_t getValue() {
    CounterValue response;
    grpc::ClientContext context;
    grpc::Status status;
    stub_->GetCounterValue(&context, google::protobuf::Empty(), &response);

    if (!status.ok()) {
      LOG(INFO) << "Failed: " << status.error_details();
    }

    return response.value();
  }

 private:
  std::unique_ptr<CounterService::Stub> stub_;
};

}
