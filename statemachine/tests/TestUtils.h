//
// Created by Rahul  Kushwaha on 6/10/23.
//
#pragma once
#include "statemachine/Common.h"
#include "statemachine/proto/RocksTxn.pb.h"

#include <fmt/format.h>

namespace rk::projects::state_machine {

struct TestUtils {

  static std::vector<RocksTxn> createNonConflictingTxns(
      std::int32_t count = 10, std::int32_t writeKeyValueSetCount = 10,
      std::int32_t readSetCount = 10, std::int32_t deleteSetCount = 10) {

    auto constexpr keyFormat = "Write key {}";
    auto constexpr valueFormat{"Write value {}"};
    auto constexpr readKeyFormat{"Read key {}"};
    auto constexpr deleteKeyFormat{"Delete key {}"};

    auto txnIdCounter{std::int64_t{0}};
    std::vector<RocksTxn> result;

    for (int i = 0; i < count; i++) {
      RocksTxn_1 txn_1{};

      // Write KeyValues
      for (int j = 0; j < writeKeyValueSetCount; j++) {
        auto keyValue = KeyValue{};
        keyValue.set_key(fmt::format(keyFormat, j));
        keyValue.set_value(fmt::format(valueFormat, j));

        txn_1.mutable_write_set()->Add(std::move(keyValue));
      }

      // Read Set
      for (int j = 0; j < readSetCount; j++) {
        txn_1.mutable_read_set()->Add(fmt::format(readKeyFormat, j));
      }

      // Delete Set
      for (int j = 0; j < deleteSetCount; j++) {
        txn_1.mutable_delete_set()->Add(fmt::format(deleteKeyFormat, j));
      }

      RocksTxn txn{.txnId = txnIdCounter++,
                   .serializedPayload = txn_1.SerializeAsString()};

      result.emplace_back(std::move(txn));
    }

    return result;
  }
};

}  // namespace rk::projects::state_machine