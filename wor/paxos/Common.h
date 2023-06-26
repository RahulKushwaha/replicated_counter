//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once

#include "folly/experimental/coro/Task.h"
#include "wor/paxos/proto/PaxosMessage.pb.h"
#include <cstdint>
#include <optional>
#include <string>

namespace rk::projects::paxos {
template <typename T> using coro = folly::coro::Task<T>;

using BallotId = internal::BallotId;
using Promise = internal::Promise;
using Proposal = internal::Proposal;

struct Ballot {
  BallotId id;
};

struct Accept {};

bool operator<=(const BallotId &x, const BallotId &y);
bool operator<(const BallotId &x, const BallotId &y);
bool operator==(const BallotId &x, const BallotId &y);

} // namespace rk::projects::paxos