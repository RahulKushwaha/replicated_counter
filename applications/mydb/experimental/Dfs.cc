//
// Created by Rahul  Kushwaha on 2/18/24.
//

#include "applications/mydb/experimental/Dfs.h"

namespace rk::projects::mydb::experimental {

Dfs::Dfs(std::shared_ptr<ResourceManager> resourceManager,
         std::shared_ptr<TransactionManager> transactionManager,
         std::shared_ptr<Transaction> txn)
    : resourceManager_{std::move(resourceManager)},
      transactionManager_{std::move(transactionManager)},
      txn_{std::move(txn)} {}

std::vector<std::shared_ptr<Transaction>> Dfs::getCycle() {
  bool cycleFound{false};
  std::vector<std::shared_ptr<Transaction>> cycle{txn_};

  getCycle(cycleFound, txn_, cycle);

  LOG(INFO) << "START DFS RUN" << std::endl;
  if (cycleFound) {
    LOG(INFO) << "Total Cycle Size: " << cycle_.size() << std::endl;
  }
  LOG(INFO) << "END DFS RUN" << std::endl;

  return cycle_;
}

void Dfs::getCycle(bool& cycleFound, const std::shared_ptr<Transaction>& txn,
                   std::vector<std::shared_ptr<Transaction>>& cycle) {
  assert(!txn->id.empty() && "empty txn id");
  if (cycleFound) {
    return;
  }

  if (auto itr = colorLookup_.emplace(txn->id, Color::Gray);
      itr.first->second == Color::Black) {
    return;
  }

  if (txn->waitedBy.has_value()) {
    auto waitingFor = txn->waitedBy.value();
    cycle.emplace_back(waitingFor);
    LOG(INFO) << txn->id << " -> " << waitingFor->id << " [" << cycle.size()
              << "] ";

    if (colorLookup_.contains(waitingFor->id) &&
        colorLookup_[waitingFor->id] == Color::Gray) {
      cycleFound = true;
      cycle_ = cycle;
      return;
    }

    getCycle(cycleFound, waitingFor, cycle);

    cycle.pop_back();
  }

  colorLookup_[txn->id] = Color::Black;
}

std::string Dfs::cycleDebugString() const {
  std::stringstream ss;
  for (int i = 0; i < cycle_.size(); i++) {
    auto txn = cycle_[i];
    ss << txn->id;
    if (i < cycle_.size() - 1) {
      ss << " -> ";
    }
  }

  return ss.str();
}

}  // namespace rk::projects::mydb::experimental