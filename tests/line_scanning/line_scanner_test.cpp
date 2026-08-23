#include "file_io/memory_reader.h"
#include "line_scanning/line_scanner.h"
#include "test_support.h"

#include <stdexcept>

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

TEST(line_scanner_centered_probe_compares_a_complete_line_and_preserves_newline_ownership) {
  find::file_io::MemoryReader reader("aaa\nbbb\nccc");
  find::line_scanning::LineScanner scanner(reader, 8);

  const auto middle = scanner.probe_containing(5, "bbb");
  REQUIRE(middle.has_value());
  REQUIRE(middle->range.start == 4);
  REQUIRE(middle->range.end == 7);
  REQUIRE(middle->ordering == std::strong_ordering::equal);

  const auto lower_term = scanner.probe_containing(5, "bba");
  REQUIRE(lower_term.has_value());
  REQUIRE(lower_term->ordering == std::strong_ordering::greater);

  const auto higher_term = scanner.probe_containing(5, "bbc");
  REQUIRE(higher_term.has_value());
  REQUIRE(higher_term->ordering == std::strong_ordering::less);

  const auto newline = scanner.probe_containing(3, "aaa");
  REQUIRE(newline.has_value());
  REQUIRE(newline->range.start == 0);
  REQUIRE(newline->range.end == 3);
  REQUIRE(newline->ordering == std::strong_ordering::equal);
}

TEST(line_scanner_centered_probe_falls_back_when_a_line_crosses_the_window_boundary) {
  find::file_io::MemoryReader reader("aaaaaaaaaa\nb");
  find::line_scanning::LineScanner scanner(reader, 4);
  REQUIRE(!scanner.probe_containing(5, "aaaaaaaaaa").has_value());
}

TEST(line_scanner_centered_probe_handles_empty_input_and_rejects_an_empty_buffer) {
  find::file_io::MemoryReader reader("");
  find::line_scanning::LineScanner scanner(reader);
  REQUIRE(!scanner.probe_containing(0, "term").has_value());

  bool threw = false;
  try {
    [[maybe_unused]] find::line_scanning::LineScanner invalid_scanner(reader, 0);
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  REQUIRE(threw);
}
