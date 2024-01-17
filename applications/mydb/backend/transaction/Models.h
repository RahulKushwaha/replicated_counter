//
// Created by Rahul  Kushwaha on 1/16/24.
//

#pragma once
#include <map>
#include <string>
#include <vector>

namespace rk::projects::mydb::transaction {

using PartitionId = std::string;

struct  Partition {
  PartitionId partitionId;
};

enum class PartitionStatus : std::uint32_t {
  UNKNOWN = 0,
  PREPARED = 1,
  COMMITTED = 2,
  ABORTED = 3,
};

enum class TransactionStatus : std::uint16_t {
  UNKNOWN = 0,
  PREPARE = 1,
  COMMIT = 2,
  ABORT = 3,
};

struct Transaction {
  std::string id;
  std::vector<Partition> participatingNodes;
  std::uint32_t totalVotes;
  std::map<PartitionId, PartitionStatus> partitionStatus;
  std::string payload;
  TransactionStatus status;
};

}  // namespace rk::projects::mydb::transaction