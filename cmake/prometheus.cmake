set(ENABLE_TESTING OFF)
FetchContent_Declare(
        prometheus
        GIT_REPOSITORY https://github.com/jupp0r/prometheus-cpp.git
        GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(prometheus)
set(ENABLE_TESTING ON)