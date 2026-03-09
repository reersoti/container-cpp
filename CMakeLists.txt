cmake_minimum_required(VERSION 3.16)
project(custom_containers_cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wconversion -Wshadow>
)

add_executable(containers_demo
    src/main.cpp
)

target_include_directories(containers_demo PRIVATE include)
target_link_libraries(containers_demo PRIVATE project_warnings)

include(CTest)
if(BUILD_TESTING)
    add_executable(smoke_tests tests/smoke_tests.cpp)
    target_include_directories(smoke_tests PRIVATE include)
    target_link_libraries(smoke_tests PRIVATE project_warnings)
    add_test(NAME smoke_tests COMMAND smoke_tests)
endif()
