//
// Created by Rahul  Kushwaha on 6/10/23.
//
#pragma once
#include "rocksdb/utilities/write_batch_with_index.h"
#include <string>

namespace rk::projects::state_machine {
using LogId = std::int64_t;

struct SpeculativeTxnResult {
  bool conflictDetected;
  bool txnSucceeded;
};

struct RocksTxnResult {
  bool txnSucceeded;
  SpeculativeTxnResult speculativeTxnResult;
};

struct SpeculativeExecution {
  std::vector<std::string> keySet;
  rocksdb::WriteBatchWithIndex batch;
  LogId logId;
};

struct RocksTxn {
  std::int64_t txnId;
  std::optional<SpeculativeExecution> speculativeExecution;
  std::string serializedPayload;
  LogId logId;
};

} // namespace rk::projects::state_machine