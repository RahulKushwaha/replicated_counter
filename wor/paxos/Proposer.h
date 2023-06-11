//
// Created by Rahul  Kushwaha on 6/3/23.
//

#pragma once
#include "folly/experimental/coro/Collect.h"
#include "folly/experimental/coro/TimedWait.h"
#include "wor/paxos/include/Acceptor.h"
#include "wor/paxos/include/Proposer1.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace rk::projects::paxos {
using PrepareResponse = std::variant<Promise, std::false_type>;

class Proposer : public Proposer1 {
public:
  explicit Proposer(std::int32_t majoritySize,
                    std::vector<std::shared_ptr<Acceptor>> acceptors)
      : majority_{majoritySize}, prepareAcceptors_{std::move(acceptors)},
        proposeAcceptors_{}, selectedProposal_{} {}

  coro<bool> prepare(BallotId ballotId) override {
    // first clear the list of acceptors from the proposeAcceptors_ list.
    proposeAcceptors_.clear();

    std::vector<coro<folly::Optional<PrepareResponse>>> prepareTasks;
    for (auto &acceptor : prepareAcceptors_) {
      auto prepareTask = acceptor->prepare(Ballot{ballotId});
      auto prepareWithTimeout = folly::coro::timed_wait(
          std::move(prepareTask), std::chrono::milliseconds(100));

      prepareTasks.emplace_back(std::move(prepareWithTimeout));
    }

    // make a function that allows for waiting for first n successful.
    auto prepares =
        co_await folly::coro::collectAllRange(std::move(prepareTasks));

    std::vector<Proposal> proposals;
    std::int32_t successfulPrepares{0};
    auto index{0};
    for (auto &prepareResp : prepares) {
      if (prepareResp.has_value() &&
          std::holds_alternative<Promise>(prepareResp.value())) {
        auto ballotIdAndValue = std::get<Promise>(prepareResp.value());
        if (ballotIdAndValue.value.has_value()) {
          proposals.emplace_back(
              Proposal{ballotIdAndValue.id, ballotIdAndValue.value.value()});
          successfulPrepares++;
        } else {
          successfulPrepares++;
        }

        proposeAcceptors_.emplace_back(prepareAcceptors_.at(index));
      }

      index++;
    }

    if (successfulPrepares < majority_) {
      co_return false;
    }

    std::optional<Proposal> selectedProposal;
    for (auto &proposal : proposals) {
      if (!selectedProposal.has_value() ||
          (selectedProposal.has_value() &&
           selectedProposal.value().id < proposal.id)) {
        selectedProposal = proposal;
      }
    }

    selectedProposal_ = selectedProposal;

    co_return true;
  }

  coro<bool> propose(BallotId ballotId, std::string value) override {
    Proposal proposal{.id = ballotId,
                      .value = selectedProposal_.has_value()
                                   ? selectedProposal_->value
                                   : value};

    std::int32_t successfulProposals{0};
    for (auto &acceptor : proposeAcceptors_) {
      auto result = co_await acceptor->accept(proposal);
      if (result) {
        successfulProposals++;
      }
    }

    if (successfulProposals < majority_) {
      co_return false;
    }

    bool committed{false};
    for (auto &acceptor : proposeAcceptors_) {
      auto commitResult = co_await acceptor->commit(ballotId);
      committed |= commitResult;
    }

    co_return committed;
  }

  coro<std::optional<std::string>> getCommittedValue() override {
    for (auto &acceptor : proposeAcceptors_) {
      if (auto committedValue = co_await acceptor->getCommittedValue();
          committedValue.has_value()) {
        co_return committedValue.value();
      }
    }

    co_return {};
  }

private:
  std::int32_t majority_;
  std::vector<std::shared_ptr<Acceptor>> prepareAcceptors_;
  std::vector<std::shared_ptr<Acceptor>> proposeAcceptors_;
  std::optional<Proposal> selectedProposal_;
};
} // namespace rk::projects::paxos