//
// Created by Rahul  Kushwaha on 1/2/23.
//

#ifndef LOGSTORAGE_REPLICAIMPL_H
#define LOGSTORAGE_REPLICAIMPL_H

#include "../include/Replica.h"
#include "../Sequencer.h"
#include "../include/NanoLogStore.h"

namespace rk::project::counter {

class ReplicaImpl: public Replica {
 public:
  explicit ReplicaImpl(std::shared_ptr<NanoLogStore> nanoLogStore,
                       std::shared_ptr<MetadataStore> metadataStore);

  std::string getId() override;
  std::string getName() override;

  folly::SemiFuture<folly::Unit>
  append(LogId logId, std::string logEntryPayload) override;
  LogId seal() override;

 private:
  std::shared_ptr<NanoLogStore> nanoLogStore_;
  std::shared_ptr<MetadataStore> metadataStore_;
};

}

#endif //LOGSTORAGE_REPLICAIMPL_H
