set(
        TEST_FILES_LIST

        log/impl/tests/MetadataStoreTest.cc
        log/impl/tests/MockReplica.h
        log/impl/tests/ReplicaTest.cc
        log/impl/tests/SequencerTest.cc
        log/impl/tests/TestUtils.h
        log/impl/tests/NanoLogTests.cc
        log/impl/tests/VirtualLogTest.cc
        log/utils/tests/FutureUtilsTest.cc
        log/utils/tests/OrderedCompletionQueueTests.cc
        log/utils/tests/UuidGeneratorTest.cc
        log/utils/tests/FlatMapSequentialTest.cc

        applications/counter/test/InMemoryFakeVirtualLog.h
        applications/counter/test/CounterAppTest.cc

        wor/tests/WriteOnceRegisterTests.cc
        wor/tests/WriteOnceRegisterChainAppenderTest.cc
        wor/paxos/tests/AcceptorTests.cc
        wor/paxos/tests/ProposerTests.cc

        statemachine/tests/RocksStateMachineTests.cc
)

list(APPEND TEST_FILES_LIST ${SOURCE_FILES_LIST})
