cmake_minimum_required(VERSION 3.23)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

include(external/uuid/CMakeLists.txt)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

add_executable(LogStorage main.cpp log/include/DurableLog.h
        applications/counter/CounterEntry.pb.h applications/counter/CounterEntry.pb.cc
        log/include/NanoLog.h log/include/Common.h log/include/MetadataStore.h
        log/impl/InMemoryMetadataStore.cc log/impl/InMemoryMetadataStore.h
        log/include/VirtualLog.h log/impl/VirtualLogImpl.h log/impl/VirtualLogImpl.cc
        log/include/Replica.h log/impl/SequencerImpl.cc log/impl/SequencerImpl.h
        log/include/Sequencer.h log/impl/VectorBasedNanoLog.cc log/impl/VectorBasedNanoLog.h
        log/utils/OrderedCompletionQueue.h log/utils/FutureUtils.h
        log/impl/ReplicaImpl.cc log/impl/ReplicaImpl.h
        log/include/NanoLogStore.h log/impl/NanoLogStoreImpl.cc log/impl/NanoLogStoreImpl.h
        applications/counter/CounterApp.cc applications/counter/CounterApp.h
        log/impl/VirtualLogFactory.h log/impl/VirtualLogFactory.cc
        log/MetadataConfig.pb.h log/MetadataConfig.pb.cc)

target_link_libraries(LogStorage folly protobuf glog)

enable_testing()

add_executable(
        tests
        log/impl/VectorBasedNanoLog.cc log/impl/VectorBasedNanoLog.h
        log/impl/tests/VectorBasedNanoLogTest.cc

        log/MetadataConfig.pb.h log/MetadataConfig.pb.cc
        log/impl/InMemoryMetadataStore.h log/impl/InMemoryMetadataStore.cc
        log/impl/tests/MetadataStoreTest.cc

        log/impl/ReplicaImpl.h log/impl/ReplicaImpl.cc
        log/impl/NanoLogStoreImpl.h log/impl/NanoLogStoreImpl.cc
        log/impl/tests/ReplicaTest.cc

        log/impl/SequencerImpl.h log/impl/SequencerImpl.cc
        log/include/Sequencer.h log/impl/VectorBasedNanoLog.h log/impl/VectorBasedNanoLog.cc
        log/impl/tests/SequencerTest.cc

        log/impl/tests/MockReplica.h

        log/impl/VirtualLogImpl.cc log/impl/VirtualLogImpl.h
        log/include/VirtualLog.h
        log/impl/tests/VirtualLogTest.cc

        log/impl/tests/TestUtils.h

        log/utils/OrderedCompletionQueue.h
        log/utils/tests/OrderedCompletionQueueTests.cc

        log/utils/FutureUtils.h
        log/utils/tests/FutureUtilsTest.cc
)

target_link_libraries(tests gtest)
target_link_libraries(tests glog)
target_link_libraries(tests gmock)
target_link_libraries(tests protobuf)
target_link_libraries(tests folly)

target_link_libraries(
        tests
        gtest_main
)
