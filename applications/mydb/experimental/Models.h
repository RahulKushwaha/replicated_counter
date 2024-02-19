//
// Created by Rahul  Kushwaha on 2/18/24.
//

#pragma once
#include <glog/logging.h>

#include <cassert>
#include <map>
#include <ostream>
#include <set>
#include <string>

namespace rk::projects::mydb::experimental {

enum class LockMode {
  NONE,
  READ,
  WRITE,
};

struct Element {
  std::string name;
  LockMode lockModeRequired;

  bool operator<(const Element& other) const {
    return std::lexicographical_compare(name.begin(), name.end(),
                                        other.name.begin(), other.name.end());
  }
};

struct Resource {
  std::string id;
  LockMode lockMode;
  std::uint32_t writersWaitingCount;
  std::set<std::string> txnsQ;

  bool isWriterAwaiting() const { return writersWaitingCount > 0; }

  bool operator<(const Resource& other) const {
    return std::lexicographical_compare(id.begin(), id.end(), other.id.begin(),
                                        other.id.end());
  }
};

class ResourceManager {
 public:
  bool registerResource(const std::string& resourceName) {
    if (auto itr = lookup_.find(resourceName); itr != lookup_.end()) {
      return false;
    }

    Resource resource{
        .id = resourceName,
        .lockMode = LockMode::NONE,
        .writersWaitingCount = 0,
        .txnsQ = {},
    };

    lookup_.emplace(resourceName,
                    std::make_shared<Resource>(std::move(resource)));

    return true;
  }

  std::optional<std::shared_ptr<Resource>> getResource(
      const std::string& resourceName) const {
    if (auto itr = lookup_.find(resourceName); itr != lookup_.end()) {
      return itr->second;
    }

    return {};
  }

 private:
  std::map<std::string, std::shared_ptr<Resource>> lookup_;
};

enum class TransactionState {
  ALL_RESOURCES_ACQUIRED,
  WAITING,
};

struct Transaction {
  std::string id;
  std::set<Element> elements;
  std::optional<std::shared_ptr<Transaction>> waitedBy;

  friend std::ostream& operator<<(std::ostream& os,
                                  const Transaction& transaction) {
    os << "id: " << transaction.id << ", elements: ";

    os << "[ ";
    for (const auto& element : transaction.elements) {
      os << element.name << " ";
    }
    os << " ] ";

    os << ", waitingFor: "
       << (transaction.waitedBy.has_value() ? transaction.waitedBy.value()->id
                                            : "NONE");
    return os;
  }
};

class TransactionManager {
 public:
  std::optional<std::shared_ptr<Transaction>> getTransaction(
      const std::string& txnId) {
    if (auto itr = lookup_.find(txnId); itr != lookup_.end()) {
      return itr->second;
    }

    return {};
  }

  bool add(std::shared_ptr<Transaction> txn) {
    auto itr = lookup_.emplace(txn->id, txn);
    LOG(INFO) << *(itr.first->second) << std::endl;
    LOG(INFO) << (itr.second ? "TRUE" : "False") << std::endl;
    return itr.second;
  }

 private:
  std::map<std::string, std::shared_ptr<Transaction>> lookup_;
};
}  // namespace rk::projects::mydb::experimental