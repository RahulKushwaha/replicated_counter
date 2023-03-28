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