cmake_minimum_required(VERSION 3.21)
project(LogStorage)
set(CMAKE_CXX_STANDARD 20)

include(external/uuid/CMakeLists.txt)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-fcoroutines-ts")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

set(CMAKE_VERBOSE_MAKEFILE ON)

MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH (child ${children})
        IF (IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF ()
    ENDFOREACH ()
    SET(${result} ${dirlist})
ENDMACRO()

include(ExternalProject)

SET(folly_coro_cmake_FOLLY_INSTALL "${CMAKE_BINARY_DIR}/folly_install")

set(FOLLY_BUILD_ARGUMENTS "--extra-cmake-defines={\"CMAKE_CXX_FLAGS\": \"-fcoroutines-ts\", \"CMAKE_C_COMPILER\": \"gcc\", \"CMAKE_CXX_COMPILER\": \"g++\"}")

ExternalProject_Add(FOLLY
        PREFIX "folly"
        GIT_REPOSITORY https://github.com/facebook/folly.git
        GIT_TAG v2023.03.06.00
        CONFIGURE_COMMAND ""
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_COMMAND cd "${CMAKE_BINARY_DIR}/folly/src/folly/build/fbcode_builder/" && python3 getdeps.py --install-prefix "${CMAKE_BINARY_DIR}/folly_install" build folly "${FOLLY_BUILD_ARGUMENTS}"
        INSTALL_DIR "${folly_coro_cmake_FOLLY_INSTALL}"
        USES_TERMINAL_BUILD 1
        USES_TERMINAL_DOWNLOAD 1
        USES_TERMINAL_CONFIGURE 1
        USES_TERMINAL_TEST 1
        )

set(CMAKE_CXX_STANDARD 20)
SUBDIRLIST(SUBDIRS ${folly_coro_cmake_FOLLY_INSTALL})

FOREACH (DIRECTORY ${SUBDIRS})
    include_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/include)
ENDFOREACH ()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach (dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach ()

FOREACH (DIRECTORY ${SUBDIRS})
    string(FIND ${DIRECTORY} "glog" out)
    if ("${out}" EQUAL 0)
        message(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/glog)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "fmt" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "libevent" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "double-conversion" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "gflags" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "folly" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()

    string(FIND ${DIRECTORY} "boost" out)
    if ("${out}" EQUAL 0)
        link_directories(${folly_coro_cmake_FOLLY_INSTALL}/${DIRECTORY}/lib/)
    endif ()
ENDFOREACH ()

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


# hack to enable protobuf
# need to add it as external_project
link_directories(/opt/homebrew/Cellar/protobuf/21.12/lib)
link_directories(/opt/homebrew/Cellar/googletest/1.13.0/lib)

add_executable(LogStorage main.cpp
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
        log/include/Registry.h applications/counter/server/CounterAppServer.h applications/counter/client/CounterAppClient.cc applications/counter/client/CounterAppClient.h log/server/ReplicaServer.cc log/server/ReplicaServer.h log/server/SequencerServer.cc log/server/SequencerServer.h log/server/MetadataServer.cc log/server/MetadataServer.h log/client/ReplicaClient.cc log/client/ReplicaClient.h log/client/SequencerClient.cc log/client/SequencerClient.h log/client/MetadataStoreClient.cc log/client/MetadataStoreClient.h applications/EnsembleNode.h log/impl/RemoteSequencer.h log/impl/RemoteReplica.h applications/CounterAppEnsembleNode.h applications/Ensemble.h log/impl/RegistryImpl.cc log/impl/RegistryImpl.h log/utils/GrpcServerFactory.h log/utils/UuidGenerator.h)

target_link_libraries(LogStorage
        folly protobuf glog
        proto-objects event
        glog gflags folly
        fmt double-conversion
        boost_context dl)

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
        log/utils/tests/UuidGeneratorTest.cc)

target_link_libraries(tests
        folly protobuf glog
        proto-objects event
        glog gflags folly
        fmt double-conversion
        gmock gtest
        boost_context dl
        gtest_main)