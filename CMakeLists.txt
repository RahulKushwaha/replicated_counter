cmake_minimum_required(VERSION 3.21)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

include(external/uuid/CMakeLists.txt)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

file(GLOB PROTO_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/log/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/log/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/server/proto/*.proto"
        )

find_package(Protobuf)
find_package(grpc)
add_library(proto-objects OBJECT ${PROTO_SOURCES})
set_target_properties(proto-objects PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto")

protobuf_generate(
        TARGET proto-objects
        OUT_VAR PROTO_GENERATED_FILES
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)

protobuf_generate(
        TARGET proto-objects
        OUT_VAR PROTO_GENERATED_FILES
        LANGUAGE grpc
        GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
        PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
        IMPORT_DIRS ${PROTO_IMPORT_DIRS}
        PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)

include_directories("$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

add_executable(LogStorage main.cpp log/include/DurableLog.h
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
        log/include/Registry.h applications/counter/server/CounterAppServer.h applications/counter/client/CounterAppClient.cc applications/counter/client/CounterAppClient.h log/server/ReplicaServer.cc log/server/ReplicaServer.h log/server/SequencerServer.cc log/server/SequencerServer.h)

target_link_libraries(LogStorage folly protobuf glog)
target_link_libraries(LogStorage proto-objects)

enable_testing()

add_executable(
        tests
        log/impl/VectorBasedNanoLog.cc log/impl/VectorBasedNanoLog.h
        log/impl/tests/VectorBasedNanoLogTest.cc

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
target_link_libraries(tests proto-objects)

target_link_libraries(
        tests
        gtest_main
)
