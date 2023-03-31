//
// Created by Rahul  Kushwaha on 3/31/23.
//
#include "log/utils/GrpcServerFactory.h"
#include "log/server/MetadataServer.h"
#include "log/impl/InMemoryMetadataStore.h"
#include "folly/executors/CPUThreadPoolExecutor.h"

auto main() -> int {
  std::string metadataServerAddress{"127.0.0.1:60003"};
  std::shared_ptr<rk::projects::durable_log::MetadataStore> metadataStore =
      std::make_shared<rk::projects::durable_log::InMemoryMetadataStore>();

  std::shared_ptr<rk::projects::durable_log::server::MetadataServer> server =
      std::make_shared<rk::projects::durable_log::server::MetadataServer>(
          std::move(metadataStore));

  std::shared_ptr<folly::CPUThreadPoolExecutor>
      executor = std::make_shared<folly::CPUThreadPoolExecutor>(2);

  rk::projects::durable_log::server::runRPCServer(metadataServerAddress,
                                                  server.get(),
                                                  executor.get(),
                                                  "Metadata Server").get();

  std::this_thread::sleep_for(std::chrono::milliseconds{
      std::numeric_limits<std::int64_t>::max()});
}