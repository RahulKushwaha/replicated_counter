//
// Created by Rahul  Kushwaha on 6/7/23.
//
#pragma once
#include "wor/include/WriteOnceRegister.h"

#include <memory>
#include <mutex>

namespace rk::projects::wor {

class InMemoryWriteOnceRegister : public WriteOnceRegister {
public:
  explicit InMemoryWriteOnceRegister()
      : lockId_{0}, committed_{false}, payload_{},
        mtx_{std::make_unique<std::mutex>()} {}

  coro<std::optional<LockId>> lock() override {
    std::lock_guard lg{*mtx_};
    lockId_++;
    co_return {lockId_};
  }

  coro<bool> write(LockId lockId, std::string payload) override {
    std::lock_guard lg{*mtx_};
    if (!committed_ && lockId == lockId_) {
      payload_ = std::move(payload);
      committed_ = true;
      co_return true;
    }

    co_return false;
  }

  coro<std::variant<std::string, ReadError>> read() override {
    if (committed_) {
      co_return {payload_};
    }

    co_return {ReadError::NOT_WRITTEN};
  }

  ~InMemoryWriteOnceRegister() override = default;

private:
  LockId lockId_;
  bool committed_;
  std::string payload_;
  std::unique_ptr<std::mutex> mtx_;
};

} // namespace rk::projects::wor