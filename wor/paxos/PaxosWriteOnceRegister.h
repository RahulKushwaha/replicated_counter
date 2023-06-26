//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once

#include "Proposer.h"
#include "wor/include/WriteOnceRegister.h"

namespace rk::projects::paxos {

class PaxosWriteOnceRegister : public wor::WriteOnceRegister {
public:
  explicit PaxosWriteOnceRegister(std::shared_ptr<Proposer> proposer)
      : majorId_{99}, lockId_{0}, proposer_{std::move(proposer)} {}

  std::optional<wor::LockId> lock() override {
    auto lockId = getNextLockId();
    BallotId ballotId{};
    ballotId.set_major_id(majorId_);
    ballotId.set_minor_id(lockId);

    auto result = proposer_->prepare(std::move(ballotId)).semi().get();

    if (!result) {
      return {};
    }

    return {lockId};
  }

  bool write(wor::LockId lockId, std::string payload) override {
    BallotId ballotId{};
    ballotId.set_major_id(majorId_);
    ballotId.set_minor_id(lockId);

    return proposer_->propose(std::move(ballotId), std::move(payload))
        .semi()
        .get();
  }

  std::variant<std::string, ReadError> read() override {
    if (auto committedValue = proposer_->getCommittedValue().semi().get();
        committedValue.has_value()) {
      return {committedValue.value()};
    }

    return ReadError::NOT_WRITTEN;
  }

private:
  wor::LockId getNextLockId() {
    lockId_++;
    return lockId_;
  }

private:
  std::int32_t majorId_;
  wor::LockId lockId_;
  std::shared_ptr<Proposer> proposer_;
};

} // namespace rk::projects::paxos