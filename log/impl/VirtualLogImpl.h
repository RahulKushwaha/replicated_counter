//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "log/include/VirtualLog.h"
#include "log/include/Sequencer.h"
#include "log/include/NanoLog.h"
#include "log/include/MetadataStore.h"
#include "log/include/Replica.h"
#include "log/include/Registry.h"

#include <vector>

namespace rk::projects::durable_log {

class VirtualLogImpl: public VirtualLog {
 public:
  explicit VirtualLogImpl(
      std::string id,
      std::string name,
      std::shared_ptr<Sequencer> sequencer,
      std::vector<std::shared_ptr<Replica>> replicaSet,
      std::shared_ptr<MetadataStore> metadataStore,
      VersionId metadataConfigVersionId,
      std::shared_ptr<Registry> registry);

 public:

  std::string getId() override;
  std::string getName() override;
  folly::SemiFuture<LogId> append(std::string logEntryPayload) override;
  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) override;
  folly::coro::Task<MetadataConfig> getCurrentConfig() override;
  folly::coro::Task<MetadataConfig>
  reconfigure(MetadataConfig targetMetadataConfig) override;
  folly::SemiFuture<LogId> sync() override;

  ~VirtualLogImpl() override = default;

 private:
  void setState(VersionId versionId);

 private:
  struct State {
    MetadataConfig metadataConfig;
    std::shared_ptr<Sequencer> sequencer;
    std::vector<std::shared_ptr<Replica>> replicaSet;
  };

 private:
  std::string id_;
  std::string name_;
  std::shared_ptr<MetadataStore> metadataStore_;
  std::unique_ptr<State> state_;
  std::shared_ptr<Registry> registry_;
};

}