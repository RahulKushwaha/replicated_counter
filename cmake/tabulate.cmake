FetchContent_Declare(
        tabulate
        GIT_REPOSITORY https://github.com/p-ranav/tabulate.git
        GIT_TAG v1.5
)
FetchContent_MakeAvailable(tabulate)

# TODO: Replace with find tabulate.
set(EXTERNAL_TABULATE_INCLUDE_DIR ${tabulate_SOURCE_DIR}/include/)
include_directories(${EXTERNAL_TABULATE_INCLUDE_DIR})
