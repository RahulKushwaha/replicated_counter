//
// Created by Rahul  Kushwaha on 2/18/24.
//

#include "WaitForGraph.h"

namespace rk::projects::mydb::experimental {

WaitForGraph::WaitForGraph(
    std::shared_ptr<ResourceManager> resourceManager,
    std::shared_ptr<TransactionManager> transactionManager)
    : resourceManager_{std::move(resourceManager)},
      transactionManager_{std::move(transactionManager)} {}

void WaitForGraph::add(std::shared_ptr<Transaction> txn,
                       std::vector<Element> resources) {
  auto transaction = *txn;
  auto elements = getNewlyAddedResource(txn, std::move(resources));

  for (auto& element : elements) {
    auto resourcePtr = getOrAddResource(element);
    auto& resource = *(resourcePtr);

    auto lockMode = resource.lockMode;

    // If nobody has any kind of lock on the resource, then
    // the first incoming transaction acquire it immediately.
    if (lockMode == LockMode::NONE) {
      resource.lockMode = element.lockModeRequired;
    }
    // If there is no transaction waiting for write mode and the
    // current lock is read only, then it can be shared
    // with incoming transactions immediately.
    else if (lockMode == LockMode::READ && !resource.isWriterAwaiting()) {
      resource.lockMode = LockMode::READ;
    }
    // If there is already a writer waiting for the resource
    // then all the incoming transactions should wait.
    else if (lockMode == LockMode::WRITE ||
             (lockMode == LockMode::READ && resource.isWriterAwaiting())) {
      // this means some other transaction has already taken a lock.
      // we need to find a transaction to wait.

      // find a target txn to wait for
      auto waitFor = getTransactionToWaitFor(element);

      assert(
          waitFor.has_value() &&
          "the resource is locked and should have a transaction to wait for");
      waitFor.value()->waitedBy = txn;
    }

    if (element.lockModeRequired == LockMode::WRITE) {
      resource.writersWaitingCount++;
    }

    resource.txnsQ.emplace(transaction.id);
  }
}

std::vector<Element> WaitForGraph::getNewlyAddedResource(
    std::shared_ptr<Transaction> txn, std::vector<Element> resources) {
  std::vector<Element> output;
  for (auto resource : resources) {
    if (!txn->elements.contains(resource)) {
      txn->elements.emplace(resource);
      output.emplace_back(std::move(resource));
    }
  }

  return output;
}

std::optional<std::shared_ptr<Transaction>>
WaitForGraph::getTransactionToWaitFor(Element element) {
  auto resource = getOrAddResource(element);
  for (const auto& txnId : resource->txnsQ) {
    auto optionalTxn = transactionManager_->getTransaction(txnId);
    assert(optionalTxn.has_value() && "txn should exist in manager");
    auto txn = optionalTxn.value();

    if (!txn->waitedBy.has_value()) {
      return txn;
    }
  }

  return {};
}

std::shared_ptr<Resource> WaitForGraph::getOrAddResource(Element element) {
  auto optionalResource = resourceManager_->getResource(element.name);
  if (!optionalResource.has_value()) {
    resourceManager_->registerResource(element.name);
    optionalResource = resourceManager_->getResource(element.name);
    assert(optionalResource.has_value() && "resource should be present");
  }

  return optionalResource.value();
}

}  // namespace rk::projects::mydb::experimental