//
// Created by Rahul  Kushwaha on 2/26/23.
//

#include "../include/Common.h"

#include "../include/MetadataStore.h"
#include "log/server/proto/Sequencer.grpc.pb.h"
#include <folly/futures/Future.h>
#include <glog/logging.h>
#include <grpc++/grpc++.h>

namespace rk::projects::durable_log::client {

class SequencerClient {
public:
  explicit SequencerClient(std::shared_ptr<grpc::Channel> channel);

  folly::SemiFuture<std::string> getId();
  folly::SemiFuture<LogId> append(std::string logEntryPayload);
  folly::SemiFuture<LogId> latestAppendPosition();
  folly::SemiFuture<bool> isAlive();

private:
  std::unique_ptr<server::SequencerService::Stub> stub_;
};

} // namespace rk::projects::durable_log::client