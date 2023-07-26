//
// Created by Rahul  Kushwaha on 6/9/23.
//
#include "wor/WriteOnceRegisterChainAppender.h"
#include "wor/WORFactory.h"
#include "wor/inmemory/InMemoryWriteOnceRegisterChain.h"
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <vector>

namespace rk::projects::wor {

TEST(WriteOnceRegisterChainAppenderTests, WriteToChain) {
  std::string value{"hello world"};
  std::shared_ptr<WriteOnceRegisterChain> chain = makeChainUsingInMemoryWor();
  WriteOnceRegisterChainAppender<std::string> appender{chain};
  appender.append(value).semi().get();
  auto optionalWorId = chain->tail();
  ASSERT_TRUE(optionalWorId.has_value());

  auto optionalWor = chain->get(optionalWorId.value());
  ASSERT_TRUE(optionalWor.has_value());

  ASSERT_EQ(value,
            std::get<std::string>(optionalWor.value()->read().semi().get()));
}

TEST(WriteOnceRegisterChainAppenderTests, WriteMultipleToChain) {
  std::shared_ptr<WriteOnceRegisterChain> chain = makeChainUsingInMemoryWor();
  WriteOnceRegisterChainAppender<std::string> appender{chain};

  std::vector<std::string> values;
  for (int i = 0; i < 100; i++) {
    auto value = fmt::format("hello world {}", i);
    auto worId = appender.append(value).semi().get();
    ASSERT_NE(worId, -1);

    auto optionalWorId = chain->tail();
    ASSERT_TRUE(optionalWorId.has_value());

    auto optionalWor = chain->get(optionalWorId.value());
    ASSERT_TRUE(optionalWor.has_value());

    ASSERT_EQ(value,
              std::get<std::string>(optionalWor.value()->read().semi().get()));
    values.emplace_back(value);
  }

  std::vector<std::string> valuesInChain;
  auto frontWorId = chain->front();
  auto endWorId = chain->tail();
  for (auto worId = frontWorId.value(); worId <= endWorId.value(); worId++) {
    auto wor = chain->get(worId);
    ASSERT_TRUE(wor.has_value());
    valuesInChain.emplace_back(
        std::get<std::string>(wor.value()->read().semi().get()));
  }

  ASSERT_EQ(values, valuesInChain);
}

} // namespace rk::projects::wor