#include "file_io/memory_reader.h"
#include "line_scanning/line_comparator.h"
#include "test_support.h"

#include <compare>

TEST(line_comparator_reads_lines_incrementally) {
  find::file_io::MemoryReader reader("apple\napricot\napri\nap");
  find::line_scanning::LineComparator comparator(reader, 2);
  REQUIRE(comparator.read_line(0, 5) == "apple");
  REQUIRE(comparator.read_line(6, 13) == "apricot");
}
