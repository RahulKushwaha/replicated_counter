set(CMAKE_CXX_FLAGS "-fcoroutines-ts")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
include(FetchContent)
FetchContent_Declare(
        folly
        GIT_REPOSITORY https://github.com/facebook/folly.git
        GIT_TAG v2024.01.01.00
)

FetchContent_MakeAvailable(folly)