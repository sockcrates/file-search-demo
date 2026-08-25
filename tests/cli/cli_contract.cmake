if(NOT DEFINED FIND_EXECUTABLE OR NOT DEFINED FIXTURE)
  message(FATAL_ERROR "FIND_EXECUTABLE and FIXTURE are required")
endif()

function(assert_cli_case name expected_result expected_stdout expected_stderr)
  if(ARGC EQUAL 4)
    execute_process(
      COMMAND "${FIND_EXECUTABLE}"
      RESULT_VARIABLE actual_result
      OUTPUT_VARIABLE actual_stdout
      ERROR_VARIABLE actual_stderr)
  else()
    execute_process(
      COMMAND "${FIND_EXECUTABLE}" "${ARGV4}" "${ARGV5}"
      RESULT_VARIABLE actual_result
      OUTPUT_VARIABLE actual_stdout
      ERROR_VARIABLE actual_stderr)
  endif()

  if(NOT "${actual_result}" STREQUAL "${expected_result}")
    message(FATAL_ERROR "${name}: expected exit ${expected_result}, got ${actual_result}")
  endif()
  if(NOT "${actual_stdout}" STREQUAL "${expected_stdout}")
    message(FATAL_ERROR "${name}: stdout did not match exactly")
  endif()
  if(NOT "${actual_stderr}" STREQUAL "${expected_stderr}")
    message(FATAL_ERROR "${name}: stderr did not match exactly")
  endif()
endfunction()

string(CONCAT usage_message "usage: find <search-term> <filename>" "\n")

assert_cli_case(match 0 "carrot\n" "" carr "${FIXTURE}")
assert_cli_case(no_match 1 "" "" zz "${FIXTURE}")
assert_cli_case(usage_error 2 "" "${usage_message}")

set(edge_fixture "${CMAKE_CURRENT_BINARY_DIR}/find-cli-edge-fixture.txt")
set(long_line_fixture "${CMAKE_CURRENT_BINARY_DIR}/find-cli-long-line-fixture.txt")
file(WRITE "${edge_fixture}" "\n\napple\napple\nbanana")
file(WRITE "${long_line_fixture}" "")
string(REPEAT "a" 10485761 long_line)
file(APPEND "${long_line_fixture}" "${long_line}\nb")

assert_cli_case(empty_term 0 "\n" "" "" "${edge_fixture}")
assert_cli_case(duplicate 0 "apple\n" "" apple "${edge_fixture}")
assert_cli_case(unterminated_final_line 0 "banana\n" "" banana "${edge_fixture}")
assert_cli_case(long_line 0 "b\n" "" az "${long_line_fixture}")

execute_process(
  COMMAND "${FIND_EXECUTABLE}" apple "${FIXTURE}.missing"
  RESULT_VARIABLE open_error_result
  OUTPUT_VARIABLE open_error_stdout
  ERROR_VARIABLE open_error_stderr)
if(NOT "${open_error_result}" STREQUAL "2")
  message(FATAL_ERROR "open_error: expected exit 2, got ${open_error_result}")
endif()
if(NOT "${open_error_stdout}" STREQUAL "")
  message(FATAL_ERROR "open_error: stdout was not empty")
endif()
if(NOT "${open_error_stderr}" MATCHES "^find: ")
  message(FATAL_ERROR "open_error: stderr did not start with the file error prefix")
endif()

file(REMOVE "${edge_fixture}" "${long_line_fixture}")
