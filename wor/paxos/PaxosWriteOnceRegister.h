//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once

#include "ProposerImpl.h"
#include "wor/include/WriteOnceRegister.h"

namespace rk::projects::paxos {

class PaxosWriteOnceRegister : public wor::WriteOnceRegister {
public:
  explicit PaxosWriteOnceRegister(std::int32_t registerId,
                                  std::shared_ptr<Proposer> proposer)
      : registerId_{std::to_string(registerId)}, majorId_{registerId},
        lockId_{0}, proposer_{std::move(proposer)} {}

  coro<std::optional<wor::LockId>> lock() override {
    auto lockId = getNextLockId();
    BallotId ballotId{};
    ballotId.set_major_id(majorId_);
    ballotId.set_minor_id(lockId);

    auto result = co_await proposer_->prepare(registerId_, std::move(ballotId));

    if (!result) {
      co_return {};
    }

    co_return {lockId};
  }

  coro<bool> write(wor::LockId lockId, std::string payload) override {
    BallotId ballotId{};
    ballotId.set_major_id(majorId_);
    ballotId.set_minor_id(lockId);

    return proposer_->propose(registerId_, std::move(ballotId),
                              std::move(payload));
  }

  coro<std::variant<std::string, ReadError>> read() override {
    if (auto committedValue =
            co_await proposer_->getCommittedValue(registerId_);
        committedValue.has_value()) {
      co_return {committedValue.value()};
    }

    co_return ReadError::NOT_WRITTEN;
  }

private:
  wor::LockId getNextLockId() {
    lockId_++;
    return lockId_;
  }

private:
  std::string registerId_;
  std::int32_t majorId_;
  wor::LockId lockId_;
  std::shared_ptr<Proposer> proposer_;
};

} // namespace rk::projects::paxos