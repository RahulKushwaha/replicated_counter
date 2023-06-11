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
      : lockId_{0}, proposer_{std::move(proposer)} {}

  std::optional<wor::LockId> lock() override {
    auto lockId = getNextLockId();
    auto result = proposer_->prepare(lockId).semi().get();

    if (!result) {
      return {};
    }

    return {lockId};
  }

  bool write(wor::LockId lockId, std::string payload) override {
    return proposer_->propose(lockId, std::move(payload)).semi().get();
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
  wor::LockId lockId_;
  std::shared_ptr<Proposer> proposer_;
};

} // namespace rk::projects::paxos