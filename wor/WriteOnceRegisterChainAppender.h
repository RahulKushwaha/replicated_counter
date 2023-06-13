//
// Created by Rahul  Kushwaha on 6/9/23.
//
#pragma once
#include "folly/experimental/coro/Task.h"
#include "wor/include/WriteOnceRegisterChain.h"

namespace rk::projects::wor {

template <typename T> class WriteOnceRegisterChainAppender {
public:
  explicit WriteOnceRegisterChainAppender(
      std::shared_ptr<WriteOnceRegisterChain> chain)
      : chain_{std::move(chain)} {}

  folly::coro::Task<WorId> append(T t) {
    bool successfullyWritten{false};
    WorId successfullyWrittenWorId{-1};
    while (!successfullyWritten) {
      auto worId = chain_->append();
      if (!worId.has_value()) {
        continue;
      }

      auto optionalWor = chain_->get(worId.value());
      if (!optionalWor.has_value()) {
        continue;
      }

      auto wor = optionalWor.value();
      while (true) {
        auto lockId = wor->lock();
        if (!lockId.has_value()) {
          continue;
        }

        auto writeResponse = wor->write(lockId.value(), t);
        if (writeResponse) {
          successfullyWritten = true;
          break;
        }

        auto readValue = wor->read();
        if (std::holds_alternative<WriteOnceRegister::ReadError>(readValue)) {
          // We either encounter an error or it is not written. In any case we
          // need to continue writing to it until it succeeds.
          continue;
        }

        if (std::get<std::string>(readValue) == t) {
          successfullyWrittenWorId = worId.value();
          successfullyWritten = true;
          break;
        }
      }
    }

    co_return successfullyWrittenWorId;
  }

private:
  std::shared_ptr<WriteOnceRegisterChain> chain_;
};

} // namespace rk::projects::wor