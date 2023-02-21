//
// Created by Rahul  Kushwaha on 2/20/23.
//

#include "VirtualLogFactory.h"
#include "VirtualLogImpl.h"
#include "SequencerImpl.h"
#include "uuid.h"

namespace rk::projects::durable_log {

namespace {

static uuids::uuid_random_generator uuidGeneratorMethod() {
  std::random_device rd;
  auto seed_data = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
  std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
  std::mt19937 generator(seq);
  uuids::uuid_random_generator gen{generator};

  return gen;
}

auto uuidGenerator = uuidGeneratorMethod();
}

std::unique_ptr<VirtualLog> makeVirtualLog(std::string name) {
  std::shared_ptr<MetadataStore> metadataStore = makeMetadataStore();

  // Add metadata block.
  {
    MetadataConfig config;
    config.set_versionid(1);
    config.set_previousversionid(0);
    config.set_startindex(1);
    config.set_endindex(1000);

    metadataStore->compareAndAppendRange(0, config);
  }

  std::vector<std::shared_ptr<Replica>> replicaSet;
  for (int i = 0; i < 5; i++) {
    auto replica = makeReplica("", "", makeNanoLogStore(), metadataStore);
    replicaSet.emplace_back(std::move(replica));
  }

  std::shared_ptr<Sequencer> sequencer = makeSequencer(replicaSet);
  std::unique_ptr<VirtualLog> virtualLog = std::make_unique<VirtualLogImpl>(
      to_string(uuidGenerator()),
      std::move(name),
      sequencer,
      replicaSet,
      metadataStore
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
  return std::make_unique<SequencerImpl>(std::move(replicaSet), 1);
}

}