cmake_minimum_required(VERSION 3.8)

# ------------------------------------------
# Project
# ------------------------------------------
project(Tori CXX)
set(TORI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ------------------------------------------
# Options
# ------------------------------------------
option(TORI_BUILD_TEST "Build tests" ON)
option(TORI_BUILD_EXAMPLE "Build examples" ON)
option(TORI_BUILD_BENCHMARK "Build benchmarks" ON)

if(TORI_BUILD_TEST OR TORI_BUILD_BENCHMARK OR TORI_BUILD_EXAMPLE)
  set(TORI_BUILD ON)
endif()

# ------------------------------------------
# Add target requirements
# ------------------------------------------
if(TORI_BUILD)
include(external/external.cmake)
endif()

# ------------------------------------------
# Add library
# ------------------------------------------
add_library(tori INTERFACE)

# ------------------------------------------
# Add include directory
# ------------------------------------------
target_include_directories(tori INTERFACE ${TORI_DIR}/include)

# ------------------------------------------
# Add target requirements
# ------------------------------------------
target_compile_features(tori INTERFACE cxx_std_17)

# ------------------------------------------
# Compiler Flags
# ------------------------------------------
if(MSVC)
  set(TORI_COMPILE_FLAGS /W4 /Zi /EHsc /std:c++17 /permissive-)
else()
  set(TORI_COMPILE_FLAGS -O0 -Wall -Wextra -g -std=c++17 -save-temps)
endif()

# ------------------------------------------
# valgrind
# ------------------------------------------
if(TORI_BUILD)
  add_custom_target(tori_valgrind
      COMMAND ${CMAKE_CTEST_COMMAND} 
          --force-new-ctest-process --test-action memcheck
  COMMAND cat "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log")
endif()

# ------------------------------------------
# Testing
# ------------------------------------------
if(TORI_BUILD)
  include (CTest)
  enable_testing()
  # add test
  function (TORI_TEST NAME LABEL)
    add_executable(${NAME} ${NAME}.cpp ${TORI_DIR}/test/Catch2.cpp)
    target_link_libraries(${NAME} PRIVATE tori ${TORI_LINK_FLAGS})
    target_link_libraries(${NAME} PRIVATE Catch2::Catch2)
    target_compile_options(${NAME} PRIVATE ${TORI_COMPILE_FLAGS})
    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES LABELS ${LABEL})
  endfunction()
endif()

# ------------------------------------------
# Example
# ------------------------------------------
if(TORI_BUILD)
  function (TORI_EXAMPLE NAME)
    add_executable(${NAME} ${NAME}.cpp)
    target_link_libraries(${NAME} PRIVATE tori ${TORI_LINK_FLAGS})
    target_compile_options(${NAME} PRIVATE ${TORI_COMPILE_FLAGS})
    add_test(NAME ${NAME} COMMAND ${NAME})
    set_tests_properties(${NAME} PROPERTIES LABELS example)
  endfunction()
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

