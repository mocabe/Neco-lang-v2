cmake_minimum_required(VERSION 3.8)

# ------------------------------------------
# Project
# ------------------------------------------
project(Tori CXX)
set(TORI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------
# Options
# ------------------------------------------
option(TORI_BUILD_TEST "Build tests" OFF)
option(TORI_BUILD_EXAMPLE "Build examples" OFF)
option(TORI_BUILD_BENCHMARK "Build benchmarks" OFF)

# ------------------------------------------
# Testing
# ------------------------------------------
if(TORI_BUILD_TEST OR TORI_BUILD_BENCHMARK OR TORI_BUILD_EXAMPLE)
  include (CTest)
  enable_testing()
  # add test
  function (Test NAME LABEL)
    add_executable(${NAME} ${NAME}.cpp)
    target_include_directories(${NAME} PUBLIC "${TORI_DIR}/test")
    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES LABELS ${LABEL})
  endfunction()
endif()

# ------------------------------------------
# Include
# ------------------------------------------
include_directories("${TORI_DIR}/include")

# ------------------------------------------
# Compiler Flags
# ------------------------------------------
if(MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++latest /permissive-")
else()
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -std=c++17")
endif()

# ------------------------------------------
# Subdirectories
# ------------------------------------------
# add test
if(TORI_BUILD_TEST)
  add_subdirectory(test)
endif()
# add example
if(TORI_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
# add benchmark
if(TORI_BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()


