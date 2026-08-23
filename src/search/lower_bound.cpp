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
  // Every line before low is less than term; high bounds the earliest candidate.
  std::uint64_t low = 0;
  std::uint64_t high = file_size;
  // best is the earliest candidate encountered so far.
  std::optional<std::uint64_t> best_start;

  while (low < high) {
    const auto midpoint = low + (high - low) / 2;
    const auto start = scanner.line_start_containing(midpoint);
    const auto comparison = comparator.compare_from(start, file_size, term);
    const auto ordering = comparison.ordering;
    if (ordering == std::strong_ordering::equal)
      return std::string(term);
    if (ordering == std::strong_ordering::less) {
      const auto end = comparison.end.value_or(scanner.line_end(start));
      const auto next = end < file_size ? end + 1 : file_size;
      if (next <= low)
        break;
      low = next;
    } else {
      best_start = start;
      if (start >= high)
        break;
      high = start;
    }
  }
  if (!best_start)
    return std::nullopt;
  const auto end = scanner.line_end(*best_start);
  return comparator.read_line(*best_start, end);
}

} // namespace
  // find::search
