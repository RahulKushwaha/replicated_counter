set(CMAKE_CXX_FLAGS "-fcoroutines-ts")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
include(FetchContent)
set(ENV{OPENSSL_ROOT_DIR} "/Users/rahulkushwaha/projects/replicated_counter/cmake-build-debug/openssl")
FetchContent_Declare(
        folly
        GIT_REPOSITORY https://github.com/facebook/folly.git
        GIT_TAG v2023.03.27.00 # release-1.10.0
)

FETCHCONTENT_MAKEAVAILABLE(folly)