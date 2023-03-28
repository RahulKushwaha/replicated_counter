cmake_minimum_required(VERSION 3.21)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_VERBOSE_MAKEFILE ON)

include(cmake/uuid_generator.cmake)
include(cmake/source_list.cmake)
include(cmake/test_files_list.cmake)
include(cmake/gtest.cmake)
include(cmake/prometheus.cmake)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-fcoroutines-ts")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
include(cmake/folly.cmake)

file(GLOB PROTO_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/log/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/log/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/server/proto/*.proto"
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

include_directories(${CMAKE_SOURCE_DIR})

# hack to enable protobuf
# need to add it as external_project
link_directories(/opt/homebrew/Cellar/protobuf/21.12/lib)

set(
        SERVER_LIBS
        boost_context
        dl
        double-conversion
        event
        fmt
        folly
        gflags
        glog
        proto-objects
        protobuf
        prometheus-cpp::pull
)

add_executable(LogStorage main.cpp ${SOURCE_FILES_LIST})
target_link_libraries(LogStorage ${SERVER_LIBS})

add_executable(LogServer log/main.cc ${SOURCE_FILES_LIST})
target_link_libraries(LogServer ${SERVER_LIBS})

enable_testing()

add_executable(tests ${TEST_FILES_LIST})

target_link_libraries(tests
        # Sorted lines below.
        boost_context
        dl
        double-conversion
        event
        fmt
        folly
        gflags
        glog
        gmock
        gtest
        gtest_main
        proto-objects
        protobuf
        )