//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "VirtualLogFactory.h"
#include "VirtualLogImpl.h"
#include "SequencerImpl.h"
#include "RegistryImpl.h"
#include "log/utils/UuidGenerator.h"

namespace rk::projects::durable_log {

std::unique_ptr<VirtualLog> makeVirtualLog(std::string name) {
  std::shared_ptr<MetadataStore> metadataStore = makeMetadataStore();

  // Add metadata block.
  MetadataConfig config;
  config.set_version_id(1);
  config.set_previous_version_id(0);
  config.set_start_index(1);
  config.set_end_index(1000);

  metadataStore->compareAndAppendRange(0, config);


  std::vector<std::shared_ptr<Replica>> replicaSet;
  for (int i = 0; i < 5; i++) {
    auto replica = makeReplica("", "", makeNanoLogStore(), metadataStore);
    replicaSet.emplace_back(std::move(replica));
  }

  std::shared_ptr<Sequencer> sequencer = makeSequencer(replicaSet);
  std::unique_ptr<VirtualLog> virtualLog = std::make_unique<VirtualLogImpl>(
      utils::UuidGenerator::instance().generate(),
      std::move(name),
      sequencer,
      replicaSet,
      metadataStore,
      config.version_id()
  );

  return virtualLog;
}

std::unique_ptr<NanoLogStore> makeNanoLogStore() {
  return std::make_unique<NanoLogStoreImpl>();
}

std::unique_ptr<MetadataStore> makeMetadataStore() {
  return std::make_unique<InMemoryMetadataStore>();
}


std::unique_ptr<Replica>
makeReplica(std::string id,
            std::string name,
            std::shared_ptr<NanoLogStore> nanoLogStore,
            std::shared_ptr<MetadataStore> metadataStore) {
  return std::make_unique<ReplicaImpl>(std::move(id),
                                       std::move(name),
                                       std::move(nanoLogStore),
                                       std::move(metadataStore));
}

std::unique_ptr<Sequencer>
makeSequencer(std::vector<std::shared_ptr<Replica>> replicaSet) {
  return std::make_unique<SequencerImpl>("", std::move(replicaSet), 1);
}

std::unique_ptr<Registry> makeRegistry() {
  return std::make_unique<RegistryImpl>();
}

}