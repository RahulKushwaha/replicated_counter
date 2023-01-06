cmake_minimum_required(VERSION 3.23)
project(LogStorage)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

add_executable(LogStorage main.cpp include/DurableLog.h
        VectorBasedDurableLog.cc VectorBasedDurableLog.h StateMachine.h
        Counter.cc Counter.h CounterApplication.cc CounterApplication.h
        CounterEntry.pb.h CounterEntry.pb.cc Sequencer.cc Sequencer.h
        include/NanoLog.h include/Common.h include/MetadataStore.h
        impl/InMemoryMetadataStore.cc impl/InMemoryMetadataStore.h
        include/VirtualLog.h impl/VirtualLogImpl.h impl/VirtualLogImpl.cc
        include/Replica.h impl/SequencerImpl.cc impl/SequencerImpl.h
        include/Sequencer.h impl/VectorBasedNanoLog.cc impl/VectorBasedNanoLog.h
        impl/tests/VectorBasedNanoLogTest.cc utils/OrderedCompletionQueue.h utils/tests/OrderedCompletionQueueTests.cc utils/FutureUtils.h utils/tests/FutureUtilsTest.cc impl/ReplicaImpl.cc impl/ReplicaImpl.h include/NanoLogStore.h impl/NanoLogStoreImpl.cc impl/NanoLogStoreImpl.h impl/tests/MetadataStoreTest.cc)

target_link_libraries(LogStorage protobuf)
target_link_libraries(LogStorage folly)

enable_testing()

add_executable(
        tests
        impl/VectorBasedNanoLog.cc impl/VectorBasedNanoLog.h
        impl/tests/VectorBasedNanoLogTest.cc

        utils/OrderedCompletionQueue.h
        utils/tests/OrderedCompletionQueueTests.cc

        utils/FutureUtils.h
        utils/tests/FutureUtilsTest.cc
)

target_link_libraries(tests gtest)
target_link_libraries(tests glog)
target_link_libraries(tests folly)

target_link_libraries(
        tests
        gtest_main
)
