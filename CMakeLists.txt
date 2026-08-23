cmake_minimum_required(VERSION 3.24)

project(find_utility VERSION 1.0.0 LANGUAGES CXX)

find_package(Doxygen QUIET)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_HARDENING "Enable supported compiler and linker hardening" ON)
option(ENABLE_SANITIZERS "Enable address and undefined behaviour sanitizers" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy when it is available" ON)

add_library(find_core
  src/cli/arguments.cpp
  src/file_io/error.cpp
  src/file_io/file_reader.cpp
  src/file_io/memory_reader.cpp
  src/line_scanning/line_scanner.cpp
  src/line_scanning/line_comparator.cpp
  src/search/lower_bound.cpp
)
target_include_directories(find_core PUBLIC src)

function(enable_strict_warnings target)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wshadow
      -Wformat=2
      -Wnull-dereference
      -Wdouble-promotion
      -Werror
    )
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(${target} PRIVATE
      -Wcast-align
      -Wold-style-cast
      -Woverloaded-virtual
      -Wimplicit-fallthrough
      -Wzero-as-null-pointer-constant
      -Wdocumentation
    )
  endif()
endfunction()

function(enable_hardening target)
  if(NOT ENABLE_HARDENING)
    return()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${target} PRIVATE -fstack-protector-strong)
  endif()

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${target} PRIVATE -D_FORTIFY_SOURCE=3)
    target_link_options(${target} PRIVATE -Wl,-z,relro,-z,now)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang" AND NOT ENABLE_SANITIZERS)
    target_compile_options(${target} PRIVATE -D_FORTIFY_SOURCE=2)
  endif()
endfunction()

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
  if(CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
  else()
    message(STATUS "clang-tidy was not found; static analysis is disabled")
  endif()
endif()

enable_strict_warnings(find_core)
enable_hardening(find_core)

if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  target_compile_options(find_core PUBLIC
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
    -fno-sanitize-recover=all
  )
  target_link_options(find_core PUBLIC -fsanitize=address,undefined -fno-sanitize-recover=all)
endif()

add_executable(find src/main.cpp)
target_link_libraries(find PRIVATE find_core)
enable_strict_warnings(find)
enable_hardening(find)

add_executable(find_benchmark benchmarks/search_benchmark.cpp)
target_link_libraries(find_benchmark PRIVATE find_core)
enable_strict_warnings(find_benchmark)
enable_hardening(find_benchmark)

if(DOXYGEN_FOUND)
  add_custom_target(docs
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
    COMMENT "Generate Doxygen API documentation"
    VERBATIM
  )
endif()

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
  enable_hardening(find_tests)
  add_test(NAME unit COMMAND find_tests)
  add_test(NAME integration-fruit COMMAND find_tests --integration $<TARGET_FILE:find>)
  add_test(NAME integration-usage-error COMMAND find)
  set_tests_properties(integration-usage-error PROPERTIES WILL_FAIL TRUE)
  add_test(NAME integration-no-match COMMAND find z ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/vegetables.txt)
  set_tests_properties(integration-no-match PROPERTIES WILL_FAIL TRUE)
  add_test(NAME integration-open-error COMMAND find word ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/missing.txt)
  set_tests_properties(integration-open-error PROPERTIES WILL_FAIL TRUE)
endif()
