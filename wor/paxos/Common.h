//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include "folly/experimental/coro/Task.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {
template <typename T> using coro = folly::coro::Task<T>;

using BallotId = std::int64_t;

struct Ballot {
  BallotId id;
};

struct Promise {
  BallotId id;
  std::optional<std::string> value;
};

struct Proposal {
  BallotId id;
  std::string value;
};

struct Accept {};

} // namespace rk::projects::paxos