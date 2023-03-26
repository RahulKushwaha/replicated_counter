FetchContent_Declare(
        prometheus
        GIT_REPOSITORY https://github.com/jupp0r/prometheus-cpp.git
        GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(prometheus)
find_package(prometheus-cpp)