cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_COMPILER g++-10)
project(shtrassen)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_definitions(-DDEBUG)

add_executable(
  matrix
  src/algorithm.cpp
)
target_include_directories(matrix PUBLIC include)

option(USE_OPENMP "Enable OpenMP support" OFF)
if(USE_OPENMP)
    find_package(OpenMP REQUIRED)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(<ваша_цель> PUBLIC OpenMP::OpenMP_CXX)
    endif()
else()
    message(STATUS "OpenMP disabled")
endif()
