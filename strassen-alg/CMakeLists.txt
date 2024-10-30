cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_COMPILER g++-10)
project(shtrassen)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_definitions(-DDEBUG)

add_executable(
  sequential
  src/algorithm.cpp
)
add_executable(
  parallel
  src/algorithm.cpp
)

target_include_directories(sequential PUBLIC include)
target_include_directories(parallel PUBLIC include)

find_package(OpenMP REQUIRED)
target_link_libraries(parallel PUBLIC OpenMP::OpenMP_CXX)
