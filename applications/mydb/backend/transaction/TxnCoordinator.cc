//
// Created by Rahul  Kushwaha on 1/14/24.
//

#include "applications/mydb/backend/transaction/TxnCoordinator.h"

#include "applications/mydb/backend/transaction/Models.h"
#include "applications/mydb/backend/transaction/TwoPhaseCommit.h"

namespace rk::projects::mydb::transaction {

coro<client::MultiTableOperationResponse> TxnCoordinator::multiTableOperation(
    const client::MultiTableOperationRequest* request) {
  std::map<PartitionId, client::MultiTableOperationRequest>
      requestsPerPartition{};
  for (auto tableRequest : request->single_table_operation_request()) {
    auto partitionId = partitionMap_->getPartition(
        tableRequest.get_row_request().table_name().name());
    assert(
        partitionId.has_value() &&
        "incomplete shardMap, could not find partition associated with table");

    auto& partitionRequests = requestsPerPartition[partitionId.value()];
    partitionRequests.mutable_single_table_operation_request()->Add(
        std::move(tableRequest));
  }

  std::map<PartitionId, PartitionStatus> partitionStatus;
  std::vector<Partition> participatingNodes{};
  for (const auto& [partitionId, value] : requestsPerPartition) {
    participatingNodes.emplace_back(Partition{.partitionId = partitionId});
    partitionStatus[partitionId] = PartitionStatus::UNKNOWN;
  }

  // All the requests belong to the single partition.
  // No need to perform distributed commit.
  if (participatingNodes.size() == 1) {}

  auto epoch = std::chrono::steady_clock::now().time_since_epoch().count();
  std::string payloadBytes;
  auto serializationResult = request->SerializeToString(&payloadBytes);

  assert(serializationResult && "serialization of the payload failed");

  Transaction transaction{
      .id = std::to_string(epoch),
      .participatingNodes = participatingNodes,
      .totalVotes = 0,
      .partitionStatus = partitionStatus,
      .payload = std::move(payloadBytes),
      .status = TransactionStatus::UNKNOWN,
  };

  auto txnCreateResult = co_await txnManager_->create(transaction);
  assert(txnCreateResult && "txn creation failed");

  TwoPhaseCommit twoPhaseCommit{transaction, txnManager_};

  co_await twoPhaseCommit.prepare();
  auto abortOrCommit = co_await twoPhaseCommit.abortOrCommit();

  switch (abortOrCommit) {
    case TwoPhaseCommit::AbortOrCommitResult::Commit:
      co_await twoPhaseCommit.commit();
      break;
    case TwoPhaseCommit::AbortOrCommitResult::Abort:
      co_await twoPhaseCommit.abort();
      break;
    default:
      assert(false && "un-reachable case for two phase commit");
  }

  // TODO(Rahul): Accumulate results from each partition and return to client.
  co_return {};
}

}  // namespace rk::projects::mydb::transaction