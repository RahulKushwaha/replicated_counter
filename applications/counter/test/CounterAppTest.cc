//
// Created by Aman Mahajan on 4/13/23.
//

#include "applications/counter/CounterApp.h"
#include "applications/counter/CounterAppStateMachine.h"
#include "applications/counter/CounterApplicator.h"
#include "applications/counter/test/InMemoryFakeVirtualLog.h"
#include <gtest/gtest.h>

namespace rk::projects::counter_app {
using namespace testing;

namespace {
std::shared_ptr<CounterApp> makeCounterApp() {
  auto mockVirtualLog = std::make_shared<InMemoryFakeVirtualLog>();
  auto stateMachine =
      std::make_shared<CounterAppStateMachine>(std::move(mockVirtualLog));
  auto counterApp = std::make_shared<CounterApp>(stateMachine);
  auto applicator = std::make_shared<CounterApplicator>(counterApp);
  stateMachine->setApplicator(std::move(applicator));

  return counterApp;
}
} // namespace

TEST(BatchUpdateTest, successWithSingleKey) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;

  CounterApp::IncrOperation op2;
  op2.key = "testKey";
  op2.incrBy = 8;

  CounterApp::DecrOperation op3;
  op3.key = "testKey";
  op3.decrBy = 8;

  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);
  operations.emplace_back(op2);
  operations.emplace_back(op3);

  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
  ASSERT_EQ(res[1].key, "testKey");
  ASSERT_EQ(res[1].val, 13);
  ASSERT_EQ(res[2].key, "testKey");
  ASSERT_EQ(res[2].val, 5);
}

TEST(BatchUpdateTest, successWithMultipleKey) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;

  CounterApp::IncrOperation op2;
  op2.key = "testKey";
  op2.incrBy = 8;

  CounterApp::DecrOperation op3;
  op3.key = "testKey";
  op3.decrBy = 8;

  CounterApp::IncrOperation op4;
  op4.key = "testKey2";
  op4.incrBy = 20;

  CounterApp::DecrOperation op5;
  op5.key = "testKey2";
  op5.decrBy = 8;

  CounterApp::DecrOperation op6;
  op6.key = "testKey2";
  op6.decrBy = 8;

  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);
  operations.emplace_back(op2);
  operations.emplace_back(op3);
  operations.emplace_back(op4);
  operations.emplace_back(op5);
  operations.emplace_back(op6);

  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
  ASSERT_EQ(res[1].key, "testKey");
  ASSERT_EQ(res[1].val, 13);
  ASSERT_EQ(res[2].key, "testKey");
  ASSERT_EQ(res[2].val, 5);
  ASSERT_EQ(res[3].key, "testKey2");
  ASSERT_EQ(res[3].val, 20);
  ASSERT_EQ(res[4].key, "testKey2");
  ASSERT_EQ(res[4].val, 12);
  ASSERT_EQ(res[5].key, "testKey2");
  ASSERT_EQ(res[5].val, 4);
}

TEST(BatchUpdateTest, successWithNoOps) {
  auto counterApp = makeCounterApp();

  std::vector<CounterApp::Operation> operations;
  auto res = counterApp->batchUpdate(operations).semi().get();

  ASSERT_EQ(res.size(), 0);
}

TEST(BatchUpdateTest, successWithSingleKeyAndSingleIncrOperation) {
  auto counterApp = makeCounterApp();

  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 5;
  std::vector<CounterApp::Operation> operations;
  operations.emplace_back(op1);

  auto res = counterApp->batchUpdate(operations).semi().get();
  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 5);
}

TEST(BatchUpdateTest, successWithSingleKeyAndHundredOperations) {
  auto counterApp = makeCounterApp();

  std::vector<CounterApp::Operation> operations;
  CounterApp::IncrOperation op1;
  op1.key = "testKey";
  op1.incrBy = 1;
  operations.emplace_back(op1);

  for (int i = 0; i <= 99; i++) {
    counterApp->batchUpdate(operations).semi().get();
  }
  auto res = counterApp->batchUpdate(operations).semi().get();
  ASSERT_EQ(res[0].key, "testKey");
  ASSERT_EQ(res[0].val, 101);
}

} // namespace rk::projects::counter_app
