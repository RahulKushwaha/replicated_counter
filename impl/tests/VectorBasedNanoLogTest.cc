//
// Created by Rahul  Kushwaha on 1/1/23.
//

#include <gtest/gtest.h>
#include "../VectorBasedNanoLog.h"

namespace rk::project::counter {

TEST(VectorBasedNanoLogTest, AppendWithoutSealDonotFail) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};

  ASSERT_NO_THROW(log.append(4, "").get()) << "No Exception";
}

TEST(VectorBasedNanoLogTest, FailedAppendsAfterSeal) {
  VectorBasedNanoLog log{"id", "name", "versionId", 4, 40, false};
  log.seal();

  ASSERT_THROW(log.append(4, "").get(), NanoLogSealedException)
                << "Exception Thrown";
}

}
