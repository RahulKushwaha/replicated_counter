//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "../include/VirtualLog.h"
#include "../include/Sequencer.h"
#include "../include/NanoLog.h"
#include "../include/MetadataStore.h"
#include "../include/Replica.h"

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
      VersionId metadataConfigVersionId);

 public:

  std::string getId() override;
  std::string getName() override;
  folly::SemiFuture<LogId> append(std::string logEntryPayload) override;
  folly::SemiFuture<std::variant<LogEntry, LogReadError>>
  getLogEntry(LogId logId) override;
  void reconfigure() override;

  ~VirtualLogImpl() override = default;

 private:
  struct State {
    MetadataConfig metadataConfig;
  };

 private:
  std::string id_;
  std::string name_;
  std::shared_ptr<Sequencer> sequencer_;
  std::vector<std::shared_ptr<Replica>> replicaSet_;
  std::shared_ptr<MetadataStore> metadataStore_;
  std::unique_ptr<State> state_;
};

}