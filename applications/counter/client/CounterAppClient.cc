//
// Created by Rahul  Kushwaha on 2/23/23.
//

#include "CounterAppClient.h"

namespace rk::projects::counter_app {

CounterAppClient::CounterAppClient(std::shared_ptr<grpc::Channel> channel) :
    stub_{CounterService::NewStub(std::move(channel))} {}


std::int64_t
CounterAppClient::incrementAndGet(std::string key, std::int64_t incrBy) {
  IncrementRequest request{};
  request.set_key(std::move(key));
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

std::int64_t
CounterAppClient::decrementAndGet(std::string key, std::int64_t decrBy) {
  DecrementRequest request{};
  request.set_key(std::move(key));
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

std::int64_t CounterAppClient::getValue(std::string key) {
  GetCounterValueRequest request;
  request.set_key(std::move(key));

  CounterValue response;
  grpc::ClientContext context;
  grpc::Status status;
  stub_->GetCounterValue(&context, request, &response);

  if (!status.ok()) {
    LOG(INFO) << "Failed: " << status.error_details();
  }

  return response.value();
}

}