FetchContent_Declare(backward
        GIT_REPOSITORY https://github.com/bombela/backward-cpp
        GIT_TAG v1.6)
FetchContent_MakeAvailable(backward)

file(GLOB_RECURSE BACKWARD_SOURCES CONFIGURE_DEPENDS src/*.cpp)