cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
include(FetchContent)
FetchContent_Declare(
        UUID
        GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
        GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(UUID)

set(EXTERNAL_UUID_INCLUDE_DIR ${uuid_SOURCE_DIR}/include/)
include_directories(${EXTERNAL_UUID_INCLUDE_DIR})
