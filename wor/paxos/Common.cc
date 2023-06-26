//
// Created by Rahul  Kushwaha on 6/25/23.
//

#include "Common.h"
namespace rk::projects::paxos {

bool operator<=(const BallotId &x, const BallotId &y) {
  if (x.major_id() == y.major_id()) {
    return x.minor_id() <= y.minor_id();
  }

  return x.major_id() <= y.major_id();
}

bool operator<(const BallotId &x, const BallotId &y) {
  if (x.major_id() == y.major_id()) {
    return x.minor_id() < y.minor_id();
  }

  return x.major_id() < y.major_id();
}

bool operator==(const BallotId &x, const BallotId &y) {
  return x.major_id() == y.major_id() && x.minor_id() == y.minor_id();
}

} // namespace rk::projects::paxos