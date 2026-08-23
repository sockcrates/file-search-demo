#include "file_io/memory_reader.h"
#include "line_scanning/line_scanner.h"
#include "test_support.h"

TEST(line_scanner_obeys_newline_and_eof_boundaries) {
  find::file_io::MemoryReader reader("one\n\ntwo\nlast");
  find::line_scanning::LineScanner scanner(reader, 2);
  REQUIRE(scanner.line_start_containing(0) == 0);
  REQUIRE(scanner.line_start_containing(2) == 0);
  REQUIRE(scanner.line_start_containing(3) == 0);
  REQUIRE(scanner.line_start_containing(4) == 4);
  REQUIRE(scanner.line_start_containing(5) == 5);
  REQUIRE(scanner.line_start_containing(reader.size()) == 9);
  REQUIRE(scanner.line_end(0) == 3);
  REQUIRE(scanner.line_end(4) == 4);
  REQUIRE(scanner.line_end(5) == 8);
  REQUIRE(scanner.line_end(9) == reader.size());
}

TEST(line_scanner_handles_line_larger_than_buffer) {
  find::file_io::MemoryReader reader("abcdefghij\nnext");
  find::line_scanning::LineScanner scanner(reader, 3);
  REQUIRE(scanner.line_start_containing(8) == 0);
  REQUIRE(scanner.line_end(0) == 10);
}
