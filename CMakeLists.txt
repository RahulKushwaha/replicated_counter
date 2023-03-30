cmake_minimum_required(VERSION 3.21)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_VERBOSE_MAKEFILE ON)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)

include(cmake/uuid_generator.cmake)
include(cmake/source_list.cmake)
include(cmake/test_files_list.cmake)
include(cmake/gtest.cmake)
include(cmake/prometheus.cmake)
include(cmake/openssl.cmake)

set(ENV{OPENSSL_ROOT_DIR} ${OPENSSL_INSTALL_DIR})
find_package(OpenSSL 1.1.1 MODULE)
if ("${OpenSSL_FOUND}" STREQUAL "FALSE")
    message(WARNING "folly cannot be included. pls build openssl first")
else ()
    include(cmake/folly.cmake)
endif ()

file(GLOB PROTO_SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/log/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/log/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/server/proto/*.proto"
        )

find_package(Protobuf REQUIRED)
find_package(grpc REQUIRED)
add_library(proto-objects STATIC ${PROTO_SOURCES})
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

set(
        SERVER_LIBS
        folly
        proto-objects
        ${PROTOBUF_LIBRARIES}
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
        folly
        gmock
        gtest
        gtest_main
        proto-objects
        ${PROTOBUF_LIBRARIES}
        )