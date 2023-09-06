cmake_minimum_required(VERSION 3.21)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_VERBOSE_MAKEFILE ON)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)
include(FetchContent)

find_package(OpenSSL MODULE REQUIRED)

include(cmake/uuid_generator.cmake)
include(cmake/source_list.cmake)
include(cmake/test_files_list.cmake)
include(cmake/gtest.cmake)
include(cmake/prometheus.cmake)
include(cmake/backward.cmake)
include(cmake/fmt.cmake)
include(cmake/folly.cmake)

file(GLOB PROTO_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/common/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/log/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/log/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/statemachine/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/wor/paxos/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/wor/paxos/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/mydb/backend/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/mydb/client/proto/*.proto"
)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(proto-objects STATIC ${PROTO_SOURCES})
set_target_properties(proto-objects PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto")

find_package(Protobuf REQUIRED)
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

include_directories(${CMAKE_SOURCE_DIR})

find_package(RocksDB CONFIG REQUIRED)
set(ROCKS_DB_LIB RocksDB::rocksdb)

set(
        SERVER_LIBS

        folly
        proto-objects
        ${PROTOBUF_LIBRARIES}
        prometheus-cpp::pull
        ${ROCKS_DB_LIB}
        ${BACKWARD_SOURCES}
        ${BACKWARD_ENABLE}
)

add_executable(LogStorage main.cpp ${SOURCE_FILES_LIST})
target_link_libraries(LogStorage ${SERVER_LIBS})

add_executable(LogServer log/main.cc ${SOURCE_FILES_LIST})
target_link_libraries(LogServer ${SERVER_LIBS})

add_executable(CounterAppServer applications/counter/main.cc ${SOURCE_FILES_LIST})
target_link_libraries(CounterAppServer ${SERVER_LIBS})

add_executable(MetadataServer metadata/main.cc ${SOURCE_FILES_LIST})
target_link_libraries(MetadataServer ${SERVER_LIBS})

find_package(Arrow REQUIRED)
find_package(ArrowDataset REQUIRED)

enable_testing()

set(
        TEST_LIBS

        folly
        gmock
        gtest
        gtest_main
        proto-objects
        ${PROTOBUF_LIBRARIES}
        prometheus-cpp::pull
        ${ROCKS_DB_LIB}

        ${BACKWARD_SOURCES}
        ${BACKWARD_ENABLE}
        Arrow::arrow_shared
        arrow_dataset_shared
)

add_executable(mydb applications/mydb/main.cc ${SOURCE_FILES_LIST} ${MY_DB_SOURCE_FILES_LIST})
target_link_libraries(mydb ${SERVER_LIBS} Arrow::arrow_shared arrow_dataset_shared ${ROCKS_DB_LIB})

add_executable(mydb_tests ${MY_DB_TEST_FILES_LIST})
target_link_libraries(mydb_tests Arrow::arrow_shared arrow_dataset_shared ${ROCKS_DB_LIB} ${TEST_LIBS})

add_executable(tests ${TEST_FILES_LIST})

target_link_libraries(tests ${TEST_LIBS})