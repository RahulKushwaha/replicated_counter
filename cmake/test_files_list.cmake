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

        persistence/tests/RocksTestFixture.h
        persistence/tests/TransactionalRocksKVStoreLiteTests.cc

        statemachine/tests/RocksStateMachineTests.cc
        statemachine/tests/VirtualLogStateMachineTests.cc
)

set(
        MY_DB_TEST_FILES_LIST

        applications/mydb/backend/Common.h
        applications/mydb/backend/KeySerializer.cc
        applications/mydb/backend/KeySerializer.h
        applications/mydb/backend/QueryExecutor.cc
        applications/mydb/backend/QueryExecutor.h
        applications/mydb/backend/QueryOptions.h
        applications/mydb/backend/RocksDbFactory.h
        applications/mydb/backend/RocksReaderWriter.cc
        applications/mydb/backend/RocksReaderWriter.h
        applications/mydb/backend/RowSerializer.h
        applications/mydb/backend/SchemaStore.h
        applications/mydb/backend/TableRow.cc
        applications/mydb/backend/TableRow.h

        applications/mydb/backend/tests/TestUtils.h
        applications/mydb/backend/tests/TestUtils.cc

        applications/mydb/backend/tests/KeySerializerTests.cc
        applications/mydb/backend/tests/RowSerializerTests.cc
        applications/mydb/backend/tests/RocksReaderWriterTests.cc
        applications/mydb/backend/tests/KeyParserTests.cc
        applications/mydb/backend/tests/QueryExecutorTests.cc
        applications/mydb/format/tests/FormatTableTests.cc
        applications/mydb/backend/tests/QueryPlannerTest.cc
)


list(APPEND MY_DB_TEST_FILES_LIST ${MY_DB_SOURCE_FILES_LIST})
list(APPEND TEST_FILES_LIST ${SOURCE_FILES_LIST})

list(APPEND TEST_FILES_LIST ${MY_DB_SOURCE_FILES_LIST})
list(APPEND TEST_FILES_LIST ${MY_DB_TEST_FILES_LIST})
