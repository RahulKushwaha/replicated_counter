//
// Created by Rahul  Kushwaha on 2/18/24.
//

#pragma once
#include "applications/mydb/experimental/Models.h"

#include <map>
#include <memory>

namespace rk::projects::mydb::experimental {

class Dfs {
 private:
  enum class Color {
    Black,
    Gray,
  };

 public:
  explicit Dfs(std::shared_ptr<ResourceManager> resourceManager,
               std::shared_ptr<TransactionManager> transactionManager,
               std::shared_ptr<Transaction> txn);

  std::vector<std::shared_ptr<Transaction>> getCycle();

  std::string cycleDebugString() const;

 private:
  void getCycle(bool& cycleFound, const std::shared_ptr<Transaction>& txn,
                std::vector<std::shared_ptr<Transaction>>& cycle);

 private:
  std::shared_ptr<ResourceManager> resourceManager_;
  std::shared_ptr<TransactionManager> transactionManager_;
  std::shared_ptr<Transaction> txn_;

  std::map<std::string, Color> colorLookup_;
  std::vector<std::shared_ptr<Transaction>> cycle_;
};

}  // namespace rk::projects::mydb::experimental