set(
        SOURCE_FILES_LIST

        applications/mydb/backend/tests/TestUtils.h
        applications/counter/CounterAppServer.h
        applications/counter/CounterAppEnsembleNode.h
        applications/counter/CounterApp.cc
        applications/counter/CounterApp.h
        applications/counter/CounterHealthCheck.cc
        applications/counter/CounterHealthCheck.h
        applications/counter/CounterAppStateMachine.cc
        applications/counter/CounterAppStateMachine.h
        applications/counter/CounterApplicator.cc
        applications/counter/CounterApplicator.h
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
        log/impl/PersistentMetadataStore.h
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
        log/impl/InMemoryNanoLog.cc
        log/impl/InMemoryNanoLog.h
        log/impl/RocksNanoLog.h
        log/impl/RocksNanoLog.cc
        log/impl/NanoLogFactory.h
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
        log/server/LogServer.h
        log/server/SequencerServer.cc
        log/server/SequencerServer.h
        log/utils/FutureUtils.h
        log/utils/GrpcServerFactory.h
        log/utils/OrderedCompletionQueue.h
        log/utils/UuidGenerator.h

        metrics/MetricsRegistry.h

        persistence/KVStoreLite.h
        persistence/RocksKVStoreLite.h
        persistence/RocksDbFactory.h
        wor/include/WriteOnceRegister.h
        wor/include/WriteOnceRegisterChain.h
        wor/WriteOnceRegisterChainImpl.h
        wor/WriteOnceRegisterChainAppender.h
        wor/inmemory/InMemoryWriteOnceRegister.h
        wor/inmemory/InMemoryWriteOnceRegisterChain.h

        wor/paxos/include/Registry.h
        wor/paxos/include/Acceptor.h
        wor/paxos/include/Proposer.h
        wor/paxos/RegistryImpl.h
        wor/paxos/LocalAcceptor.h
        wor/paxos/LocalAcceptor.cc
        wor/paxos/Common.h
        wor/paxos/Common.cc
        wor/paxos/PaxosWriteOnceRegister.h
        wor/WORFactory.h
        wor/WORFactory.cc
        wor/paxos/server/Acceptor.h
        wor/paxos/client/AcceptorClient.h
        wor/paxos/RemoteAcceptor.h

        statemachine/include/StateMachine.h
        statemachine/RocksStateMachine.h
        statemachine/ConflictDetector.h
        statemachine/RocksTxnApplicator.h
        statemachine/MetadataStoreStateMachine.h
        statemachine/MetadataStoreApplicator.h
        statemachine/LogTrimApplicator.h
        statemachine/LogTrimStateMachine.h
        statemachine/VirtualLogStateMachine.h
        statemachine/Factory.h
        statemachine/Common.h
)