FetchContent_Declare(
        uuid
        GIT_REPOSITORY https://github.com/mariusbancila/stduuid.git
        GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(uuid)

# TODO: Replace with find package.
set(EXTERNAL_UUID_INCLUDE_DIR ${uuid_SOURCE_DIR}/include/)
include_directories(${EXTERNAL_UUID_INCLUDE_DIR})
