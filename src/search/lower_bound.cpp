#include "search/lower_bound.h"

#include "line_scanning/line_comparator.h"
#include "line_scanning/line_scanner.h"

namespace find::search {

std::optional<std::string> lower_bound(const file_io::Reader &reader, std::string_view term) {
  const auto file_size = reader.size();
  if (file_size == 0)
    return std::nullopt;

  line_scanning::LineScanner scanner(reader);
  line_scanning::LineComparator comparator(reader);
  std::uint64_t low = 0;
  std::uint64_t high = file_size;
  std::optional<line_scanning::LineRange> best;

  while (low < high) {
    const auto midpoint = low + (high - low) / 2;
    const auto line = scanner.line_containing(midpoint);
    const auto ordering = comparator.compare(line.start, line.end, term);
    if (ordering == std::strong_ordering::less) {
      const auto next = line.end < file_size ? line.end + 1 : file_size;
      if (next <= low)
        break;
      low = next;
    } else {
      best = line;
      if (line.start >= high)
        break;
      high = line.start;
    }
  }
  if (!best)
    return std::nullopt;
  return comparator.read_line(best->start, best->end);
}

} // namespace
  // find::search
