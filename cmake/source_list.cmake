set(
        SOURCE_FILES_LIST

        applications/CounterAppEnsembleNode.h
        applications/counter/CounterApp.cc
        applications/counter/CounterApp.h
        applications/counter/CounterHealthCheck.cc
        applications/counter/CounterHealthCheck.h
        applications/counter/client/CounterAppClient.cc
        applications/counter/client/CounterAppClient.h
        applications/counter/server/CounterAppServer.h
        log/client/MetadataStoreClient.cc
        log/client/MetadataStoreClient.h
        log/client/ReplicaClient.cc
        log/client/ReplicaClient.h
        log/client/SequencerClient.cc
        log/client/SequencerClient.h
        log/impl/Ensemble.h
        log/impl/EnsembleConfig.h
        log/impl/EnsembleNode.h
        log/impl/FailureDetectorImpl.cc
        log/impl/FailureDetectorImpl.h
        log/impl/InMemoryMetadataStore.cc
        log/impl/InMemoryMetadataStore.h
        log/impl/NanoLogStoreImpl.cc
        log/impl/NanoLogStoreImpl.h
        log/impl/NullSequencer.h
        log/impl/RegistryImpl.cc
        log/impl/RegistryImpl.h
        log/impl/RemoteMetadataStore.h
        log/impl/RemoteReplica.h
        log/impl/RemoteSequencer.h
        log/impl/ReplicaImpl.cc
        log/impl/ReplicaImpl.h
        log/impl/SequencerImpl.cc
        log/impl/SequencerImpl.h
        log/impl/VectorBasedNanoLog.cc
        log/impl/VectorBasedNanoLog.h
        log/impl/VirtualLogFactory.cc
        log/impl/VirtualLogFactory.h
        log/impl/VirtualLogImpl.cc
        log/impl/VirtualLogImpl.h
        log/include/Common.h
        log/include/FailureDetector.h
        log/include/HealthCheck.h
        log/include/MetadataStore.h
        log/include/NanoLog.h
        log/include/NanoLogStore.h
        log/include/Registry.h
        log/include/Replica.h
        log/include/Sequencer.h
        log/include/VirtualLog.h
        log/server/AdminServer.cc
        log/server/AdminServer.h
        log/server/MetadataServer.cc
        log/server/MetadataServer.h
        log/server/ReplicaServer.cc
        log/server/ReplicaServer.h
        log/server/RunServer.h
        log/server/SequencerServer.cc
        log/server/SequencerServer.h
        log/utils/FutureUtils.h
        log/utils/GrpcServerFactory.h
        log/utils/OrderedCompletionQueue.h
        log/utils/UuidGenerator.h
        metrics/MetricsRegistry.h
)