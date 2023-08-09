//
// Created by Rahul  Kushwaha on 3/31/23.
//
#include "folly/executors/CPUThreadPoolExecutor.h"
#include "log/impl/InMemoryMetadataStore.h"
#include "log/server/MetadataServer.h"
#include "log/utils/GrpcServerFactory.h"

auto main() -> int {
  std::string metadataServerAddress{"127.0.0.1:60003"};
  std::shared_ptr<rk::projects::durable_log::MetadataStore> metadataStore =
      std::make_shared<rk::projects::durable_log::InMemoryMetadataStore>();

  rk::projects::durable_log::MetadataConfig config{};
  config.set_version_id(1);
  config.set_previous_version_id(0);
  config.set_start_index(1);
  config.set_end_index(1000);

  metadataStore->compareAndAppendRange(config).semi().get();

  std::shared_ptr<rk::projects::durable_log::server::MetadataServer> server =
      std::make_shared<rk::projects::durable_log::server::MetadataServer>(
          std::move(metadataStore));

  std::shared_ptr<folly::CPUThreadPoolExecutor> executor =
      std::make_shared<folly::CPUThreadPoolExecutor>(2);

  rk::projects::durable_log::server::runRPCServer(
      metadataServerAddress, {server.get()}, executor.get(), "Metadata Server")
      .get();

  std::this_thread::sleep_for(
      std::chrono::milliseconds{std::numeric_limits<std::int64_t>::max()});
}