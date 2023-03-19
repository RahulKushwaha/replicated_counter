//
// Created by Rahul  Kushwaha on 3/11/23.
//

#pragma once
#include <optional>
#include "log/proto/MetadataConfig.pb.h"
#include "folly/experimental/coro/Task.h"

namespace rk::projects::durable_log {

class FailureDetector {
 public:
  virtual std::optional<MetadataConfig> getLatestMetadataConfig() = 0;

  virtual bool failure() = 0;
  /*
   * The reconciliation loop is as follows:
   *  INIT: Get the latest metadata version.
   *  1. Detect failure.
   *  2. Call reconfigure and install the new metadata until the version of the
   *  newly installed medata is greater than to the current.
   *  3. Update local metadata version.
   */
  virtual folly::coro::Task<void> reconcile() = 0;
};

}
