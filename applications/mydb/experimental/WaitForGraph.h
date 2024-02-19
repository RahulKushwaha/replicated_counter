//
// Created by Rahul  Kushwaha on 2/18/24.
//

#pragma once
#include "applications/mydb/experimental/Models.h"

#include <memory>

namespace rk::projects::mydb::experimental {

class WaitForGraph {
 public:
  WaitForGraph(std::shared_ptr<ResourceManager> resourceManager,
               std::shared_ptr<TransactionManager> transactionManager);

  void add(std::shared_ptr<Transaction> txn, std::vector<Element> resources);

 private:
  std::vector<Element> getNewlyAddedResource(std::shared_ptr<Transaction> txn,
                                             std::vector<Element> resources);
  std::optional<std::shared_ptr<Transaction>> getTransactionToWaitFor(
      Element element);
  std::shared_ptr<Resource> getOrAddResource(Element element);

 private:
  std::shared_ptr<ResourceManager> resourceManager_;
  std::shared_ptr<TransactionManager> transactionManager_;
};

}  // namespace rk::projects::mydb::experimental