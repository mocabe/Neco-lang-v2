cmake_minimum_required(VERSION 3.8)

# ------------------------------------------
# Project
# ------------------------------------------
project(Neco-lang-v2 CXX)
set(NECO_LANG_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------
# Options
# ------------------------------------------
option(NECO_LANG_BUILD_TEST "Build tests" OFF)
option(NECO_LANG_BUILD_EXAMPLE "Build examples" OFF)
option(NECO_LANG_BUILD_BENCHMARK "Build benchmarks" OFF)

# ------------------------------------------
# Testing
# ------------------------------------------
if(NECO_LANG_BUILD_TEST OR NECO_LANG_BUILD_BENCHMARK OR NECO_LANG_BUILD_EXAMPLE)
  include (CTest)
  enable_testing()
  # add test
  function (Test NAME LABEL)
    add_executable(${NAME} ${NAME}.cpp)
    target_include_directories(${NAME} PUBLIC "${NECO_LANG_DIR}/test")
    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES LABELS ${LABEL})
  endfunction()
endif()

# ------------------------------------------
# Include
# ------------------------------------------
include_directories("${PROJECT_SOURCE_DIR}/include")

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
if(NECO_LANG_BUILD_TEST)
  add_subdirectory(test)
endif()
# add example
if(NECO_LANG_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
# add benchmark
if(NECO_LANG_BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()


