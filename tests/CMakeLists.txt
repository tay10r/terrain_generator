cmake_minimum_required(VERSION 3.14.7)

include(FetchContent)

FetchContent_Declare(googletest
  URL "https://github.com/google/googletest/archive/master.zip")

FetchContent_MakeAvailable(googletest)

add_executable(tests
  ExprTests.h
  ExprTests.cpp
  CpuBackend.cpp)

if(NOT MSVC)
  target_compile_options(tests PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_link_libraries(tests PRIVATE gtest gtest_main mapgen)

add_test(NAME tests COMMAND $<TARGET_FILE:tests>)

set_target_properties(tests
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
    OUTPUT_NAME run_tests)

enable_testing()
