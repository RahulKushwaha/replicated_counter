set(
        TEST_FILES_LIST

        log/impl/FailureDetectorImpl.cc
        log/impl/FailureDetectorImpl.h
        log/impl/InMemoryMetadataStore.cc
        log/impl/InMemoryMetadataStore.h
        log/impl/NanoLogStoreImpl.cc
        log/impl/NanoLogStoreImpl.h
        log/impl/RegistryImpl.cc
        log/impl/RegistryImpl.h
        log/impl/RemoteMetadataStore.h
        log/impl/ReplicaImpl.cc
        log/impl/ReplicaImpl.h
        log/impl/SequencerImpl.cc
        log/impl/SequencerImpl.h
        log/impl/VectorBasedNanoLog.cc
        log/impl/VectorBasedNanoLog.h
        log/impl/VirtualLogImpl.cc
        log/impl/VirtualLogImpl.h
        log/impl/tests/MetadataStoreTest.cc
        log/impl/tests/MockReplica.h
        log/impl/tests/ReplicaTest.cc
        log/impl/tests/SequencerTest.cc
        log/impl/tests/TestUtils.h
        log/impl/tests/VectorBasedNanoLogTest.cc
        log/impl/tests/VirtualLogTest.cc
        log/include/FailureDetector.h
        log/include/Sequencer.h
        log/include/VirtualLog.h
        log/server/LogServer.h
        log/utils/FutureUtils.h
        log/utils/OrderedCompletionQueue.h
        log/utils/tests/FutureUtilsTest.cc
        log/utils/tests/OrderedCompletionQueueTests.cc
        log/utils/tests/UuidGeneratorTest.cc

        applications/mydb/backend/AddTableRowRequest.h
        applications/mydb/backend/Common.h
        applications/mydb/backend/KeySerializer.h
        applications/mydb/backend/KeySerializer.cc
        applications/mydb/backend/RowSerializer.h
        applications/mydb/backend/TableRow.h
        applications/mydb/backend/TableRow.cc
        applications/mydb/backend/tests/TestUtils.h
        applications/mydb/backend/tests/TestUtils.cc

        applications/mydb/backend/RocksReaderWriter.h
        applications/mydb/backend/RocksReaderWriter.cc

        applications/mydb/backend/tests/KeySerializerTests.cc
        applications/mydb/backend/tests/RowSerializerTests.cc
        applications/mydb/backend/tests/RocksReaderWriterTests.cc
)

