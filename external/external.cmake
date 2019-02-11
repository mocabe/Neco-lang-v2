message(STATUS "initializing submodule: Catch2 ...")
execute_process(COMMAND git submodule update --init -- external/Catch2
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(external/Catch2)