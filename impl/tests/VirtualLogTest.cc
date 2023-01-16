//
// Created by Rahul  Kushwaha on 1/16/23.
//

#include <gtest/gtest.h>
#include "TestUtils.h"
#include "../../include/VirtualLog.h"
#include "../VirtualLogImpl.h"

namespace rk::project::counter {

TEST(VirtualLogTests, AppendOneLogEntry) {
  auto sequencerCreationResult = createSequencer(0);
  std::shared_ptr<VirtualLog>
      log = std::make_shared<VirtualLogImpl>("virtual_log_id",
                                             "virtual_log_name",
                                             sequencerCreationResult.sequencer,
                                             sequencerCreationResult.replicaSet,
                                             sequencerCreationResult.metadataStore);

  ASSERT_EQ(log->append("hello_world").get(), 1);
}

}