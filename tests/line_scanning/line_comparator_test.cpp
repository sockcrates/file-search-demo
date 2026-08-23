#include "file_io/memory_reader.h"
#include "line_scanning/line_comparator.h"
#include "test_support.h"

#include <compare>

TEST(line_comparator_handles_prefixes) {
  find::file_io::MemoryReader reader("apple\napricot\napri\nap");
  find::line_scanning::LineComparator comparator(reader, 2);
  REQUIRE(comparator.compare(0, 5, "apri") == std::strong_ordering::less);
  REQUIRE(comparator.compare(6, 13, "apri") == std::strong_ordering::greater);
  REQUIRE(comparator.compare(14, 18, "apri") == std::strong_ordering::equal);
  REQUIRE(comparator.compare(19, 21, "apri") == std::strong_ordering::less);
}

TEST(line_comparator_handles_empty_and_long_lines) {
  find::file_io::MemoryReader reader("\naaaaaaaaaa");
  find::line_scanning::LineComparator comparator(reader, 3);
  REQUIRE(comparator.compare(0, 0, "") == std::strong_ordering::equal);
  REQUIRE(comparator.compare(0, 0, "a") == std::strong_ordering::less);
  REQUIRE(comparator.compare(1, 11, "aaaaaaaaaa") == std::strong_ordering::equal);
}
