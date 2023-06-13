//
// Created by Rahul  Kushwaha on 6/7/23.
//

#pragma once
#include "wor/include/WriteOnceRegisterChain.h"
#include "wor/inmemory/InMemoryWriteOnceRegister.h"

#include <atomic>
#include <map>
#include <mutex>

namespace rk::projects::wor {

class InMemoryWriteOnceRegisterChain : public WriteOnceRegisterChain {
public:
  explicit InMemoryWriteOnceRegisterChain()
      : worId_{0}, lookup_{}, mtx_{std::make_unique<std::mutex>()} {}

  std::optional<WorId> append() override {
    std::lock_guard lg{*mtx_};

    worId_++;
    auto wor = std::make_shared<InMemoryWriteOnceRegister>();
    lookup_.emplace(worId_, wor);
    return {worId_};
  }

  std::optional<std::shared_ptr<WriteOnceRegister>> get(WorId id) override {
    std::lock_guard lg{*mtx_};

    auto itr = lookup_.find(id);
    if (itr != lookup_.end()) {
      return itr->second;
    }

    return {};
  }

  std::optional<WorId> tail() override {
    std::lock_guard lg{*mtx_};
    if (worId_ == 0) {
      return {};
    }

    return {worId_};
  }

  std::optional<WorId> front() override {
    std::lock_guard lg{*mtx_};
    if (lookup_.empty()) {
      return {worId_};
    }

    return {lookup_.begin()->first};
  }

private:
  WorId worId_;
  std::map<WorId, std::shared_ptr<WriteOnceRegister>> lookup_;
  std::unique_ptr<std::mutex> mtx_;
};

} // namespace rk::projects::wor
