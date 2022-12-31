//
// Created by Rahul  Kushwaha on 12/31/22.
//

#ifndef LOGSTORAGE_SEQUENCERIMPL_H
#define LOGSTORAGE_SEQUENCERIMPL_H
#include "../include/Sequencer.h"
#include "../include/Replica.h"

#include <vector>

namespace rk::project::counter {

class SequencerImpl: public Sequencer {
 public:
  explicit SequencerImpl(std::vector<std::shared_ptr<Replica>> replicaSet,
                         LogId seedSeqNum);

 public:
  LogId append(std::string logEntryPayload) override;
  ~SequencerImpl() override = default;

 private:
  std::vector<std::shared_ptr<Replica>> replicaSet_;
  std::atomic<LogId> sequenceNum_;
};

}

#endif //LOGSTORAGE_SEQUENCERIMPL_H
