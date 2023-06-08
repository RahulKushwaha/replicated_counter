//
// Created by Rahul  Kushwaha on 6/7/23.
//
#include "wor/include/WriteOnceRegister.h"

#include <memory>
#include <mutex>

namespace rk::projects::wor {
class InMemoryWriteOnceRegister: public WriteOnceRegister {
 public:
  explicit InMemoryWriteOnceRegister()
      : lockId_{0}, committed_{false},
        payload_{},
        mtx_{std::make_unique<std::mutex>()} {}

  std::optional<LockId> lock() override {
    std::lock_guard lg{*mtx_};
    lockId_++;
    return {lockId_};
  }

  bool write(LockId lockId, std::string payload) override {
    std::lock_guard lg{*mtx_};
    if (!committed_ && lockId == lockId_) {
      payload_ = std::move(payload);
      committed_ = true;
      return true;
    }

    return false;
  }

  std::variant<std::string, ReadError> read() override {
    if (committed_) {
      return {payload_};
    }

    return {ReadError::NOT_WRITTEN};
  }

 private:
  LockId lockId_;
  bool committed_;
  std::string payload_;
  std::unique_ptr<std::mutex> mtx_;
};

}