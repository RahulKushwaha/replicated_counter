//
// Created by Rahul  Kushwaha on 12/30/22.
//
#include "../include/VirtualLog.h"
#include "../include/Sequencer.h"
#include "../include/NanoLog.h"
#include "../include/MetadataStore.h"
#include "../include/Replica.h"

#include <vector>

namespace rk::project::counter {

class VirtualLogImpl: public VirtualLog {
 public:
  explicit VirtualLogImpl(
      std::string id,
      std::string name,
      std::shared_ptr<Sequencer> sequencer,
      std::vector<std::shared_ptr<Replica>> replicaSet,
      std::shared_ptr<MetadataStore> metadataStore);

 public:

  std::string getId() override;
  std::string getName() override;
  LogId append(std::string logEntryPayload) override;
  std::variant<LogEntry, LogReadError> getLogEntry(LogId logId) override;
  void reconfigure() override;

 private:

 private:
  std::string id_;
  std::string name_;
  std::shared_ptr<Sequencer> sequencer_;
  std::vector<std::shared_ptr<Replica>> replicaSet_;
  std::shared_ptr<MetadataStore> metadataStore_;
};

}