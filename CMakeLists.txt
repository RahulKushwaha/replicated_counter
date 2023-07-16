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
include(cmake/backward.cmake)
include(cmake/fmt.cmake)

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
        "${CMAKE_CURRENT_LIST_DIR}/statemachine/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/wor/paxos/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/counter/server/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/mydb/backend/proto/*.proto"
        "${CMAKE_CURRENT_LIST_DIR}/applications/mydb/client/proto/*.proto"
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

find_package(Rocksdb REQUIRED)
set(ROCKS_DB_LIB /opt/homebrew/Cellar/rocksdb/8.0.0/lib/librocksdb.a)

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
)

option(MYDB "ENABLE mydb" OFF)
if (MYDB)
    find_package(Arrow REQUIRED)

    set(
            MY_DB_SOURCE_FILES_LIST

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
    )

    add_executable(mydb applications/mydb/main.cc ${SOURCE_FILES_LIST} ${MY_DB_SOURCE_FILES_LIST})
    target_link_libraries(mydb ${SERVER_LIBS} Arrow::arrow_shared ${ROCKS_DB_LIB})

    add_executable(mydb_tests ${MY_DB_TEST_FILES_LIST})
    target_link_libraries(mydb_tests Arrow::arrow_shared ${ROCKS_DB_LIB} ${TEST_LIBS})
endif ()
unset(MYDB CACHE)

add_executable(tests ${TEST_FILES_LIST})

target_link_libraries(tests ${TEST_LIBS})