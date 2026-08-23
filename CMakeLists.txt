cmake_minimum_required(VERSION 3.24)

project(find_utility VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SANITIZERS "Enable address and undefined behaviour sanitizers" OFF)

add_library(find_core
  src/cli/arguments.cpp
  src/file_io/file_reader.cpp
  src/file_io/memory_reader.cpp
  src/line_scanning/line_scanner.cpp
  src/line_scanning/line_comparator.cpp
  src/search/lower_bound.cpp
)
target_include_directories(find_core PUBLIC src)

function(enable_strict_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
  endif()
endfunction()

enable_strict_warnings(find_core)

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  target_compile_options(find_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(find_core PUBLIC -fsanitize=address,undefined)
endif()

add_executable(find src/main.cpp)
target_link_libraries(find PRIVATE find_core)
enable_strict_warnings(find)

include(CTest)
if(BUILD_TESTING)
  add_executable(find_tests
    tests/test_main.cpp
    tests/cli/arguments_test.cpp
    tests/file_io/readers_test.cpp
    tests/line_scanning/line_scanner_test.cpp
    tests/line_scanning/line_comparator_test.cpp
    tests/search/lower_bound_test.cpp
  )
  target_link_libraries(find_tests PRIVATE find_core)
  target_include_directories(find_tests PRIVATE tests)
  target_compile_definitions(find_tests PRIVATE TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  enable_strict_warnings(find_tests)
  add_test(NAME unit COMMAND find_tests)
  add_test(NAME integration-fruit COMMAND find_tests --integration $<TARGET_FILE:find>)
endif()
