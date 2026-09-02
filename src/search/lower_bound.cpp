#include "search/lower_bound.h"

#include "line_scanning/line_comparator.h"
#include "line_scanning/line_scanner.h"

#include <algorithm>
#include <limits>

namespace find::search {
namespace {

std::optional<std::string> lower_bound_contiguous(std::string_view bytes, std::string_view term) {
  if (bytes.empty())
    return std::nullopt;

  const auto size = bytes.size();
  std::size_t low = 0;
  std::size_t high = size;
  std::optional<std::size_t> best_start;

  while (low < high) {
    const auto midpoint = low + (high - low) / 2;
    const auto previous_newline =
        midpoint == 0 ? std::string_view::npos : bytes.rfind('\n', midpoint - 1);
    const auto start = previous_newline == std::string_view::npos ? 0 : previous_newline + 1;
    const auto line_end = bytes.find('\n', start);
    const auto end = line_end == std::string_view::npos ? size : line_end;
    const auto ordering = bytes.substr(start, end - start) <=> term;
    if (ordering == std::strong_ordering::equal)
      return std::string(term);
    if (ordering == std::strong_ordering::less) {
      low = end < size ? end + 1 : size;
    } else {
      best_start = start;
      high = start;
    }
  }
  if (!best_start)
    return std::nullopt;
  const auto line_end = bytes.find('\n', *best_start);
  const auto end = line_end == std::string_view::npos ? size : line_end;
  return std::string(bytes.substr(*best_start, end - *best_start));
}

} // namespace

std::optional<std::string> lower_bound(const file_io::Reader &reader, std::string_view term) {
  const auto file_size = reader.size();
  const auto bytes = reader.bytes();
  const auto size_matches =
      file_size <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) &&
      bytes.size() == static_cast<std::size_t>(file_size);
  if (size_matches)
    return lower_bound_contiguous(bytes, term);
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
    const auto probe = scanner.probe_containing(midpoint, term);
    const auto start = probe ? probe->range.start : scanner.line_start_containing(midpoint);
    const auto comparison =
        probe ? line_scanning::LineComparison{} : comparator.compare_from(start, file_size, term);
    const auto ordering = probe ? probe->ordering : comparison.ordering;
    if (ordering == std::strong_ordering::equal)
      return std::string(term);
    if (ordering == std::strong_ordering::less) {
      // line_start_containing() has already read up to midpoint. When comparison
      // ends early, continue from midpoint to avoid revisiting that prefix. A
      // midpoint at a newline is itself the current line's end.
      const auto end =
          probe ? probe->range.end : comparison.end.value_or(scanner.line_end(midpoint));
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
