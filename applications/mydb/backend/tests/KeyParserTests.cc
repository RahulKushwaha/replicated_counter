//
// Created by Rahul  Kushwaha on 4/11/23.
//
#include "applications/mydb/backend/KeySerializer.h"

#include <gtest/gtest.h>

namespace rk::projects::mydb {

TEST(KeyParserTests, parseStringWithSingleEscapeCharacter) {
  std::vector<std::string> inputs{"hello/world", "/", "/hello", "hello/"};

  for (const auto& input : inputs) {
    auto str = prefix::escapeString(input);
    std::size_t index = 0;
    auto out = prefix::parse(str, index);

    ASSERT_EQ(input, out);
  }
}

TEST(KeyParserTests, parseStringWithoutEscapeCharacter) {
  std::string input{"helloworld"};
  auto str = prefix::escapeString(input);
  std::size_t index = 0;
  auto out = prefix::parse(str, index);

  ASSERT_EQ(input, out);
}

TEST(KeyParserTests, parseStringWithMultipleEscapeCharacters) {
  std::vector<std::string> inputs{"hel/lo/world",
                                  "/",
                                  "///",
                                  "/hello/",
                                  "hello/",
                                  "",
                                  "/a/d/d/a/s/a//a/s////a/a"};

  for (const auto& input : inputs) {
    auto str = prefix::escapeString(input);
    std::size_t index = 0;
    auto out = prefix::parse(str, index);

    ASSERT_EQ(input, out);
  }
}

}  // namespace rk::projects::mydb