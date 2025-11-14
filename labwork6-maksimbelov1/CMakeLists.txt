cmake_minimum_required(VERSION 3.14)

project(labwork6-maksimbelov1)

set(CMAKE_CXX_STANDARD 23)


include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                        GIT_TAG 1.11.0)
FetchContent_MakeAvailable(cpr)


FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git
                    GIT_TAG v3.11.3)
FetchContent_MakeAvailable(json)


add_subdirectory(bin)
add_subdirectory(lib)

enable_testing()
add_subdirectory(tests)



