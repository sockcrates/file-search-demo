#include "search/lower_bound.h"

#include "file_io/contiguous_reader.h"
#include "line_scanning/line_comparator.h"
#include "line_scanning/line_scanner.h"

namespace find::search {
namespace {

std::optional<std::string> lower_bound_contiguous(std::span<const std::byte> bytes,
                                                  std::string_view term) {
  if (bytes.empty())
    return std::nullopt;

  const auto *data = reinterpret_cast<const char *>(bytes.data());
  const auto size = bytes.size();
  std::size_t low = 0;
  std::size_t high = size;
  std::optional<std::size_t> best_start;

  while (low < high) {
    const auto midpoint = low + (high - low) / 2;
    auto start = midpoint;
    while (start != 0 && data[start - 1] != '\n')
      --start;
    auto end = start;
    while (end < size && data[end] != '\n')
      ++end;
    const std::string_view line(data + start, end - start);
    const auto ordering = line <=> term;
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
  auto end = *best_start;
  while (end < size && data[end] != '\n')
    ++end;
  return std::string(data + *best_start, end - *best_start);
}

} // namespace

std::optional<std::string> lower_bound(const file_io::Reader &reader, std::string_view term) {
  if (const auto *contiguous = dynamic_cast<const file_io::ContiguousReader *>(&reader))
    return lower_bound_contiguous(contiguous->bytes(), term);
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
    const auto probe = scanner.probe_containing(midpoint, term);
    const auto start = probe ? probe->range.start : scanner.line_start_containing(midpoint);
    const auto comparison = probe ? std::optional<line_scanning::LineComparison>{}
                                  : std::optional{comparator.compare_from(start, file_size, term)};
    const auto ordering = probe ? probe->ordering : comparison->ordering;
    if (ordering == std::strong_ordering::equal)
      return std::string(term);
    if (ordering == std::strong_ordering::less) {
      const auto end = probe ? probe->range.end : comparison->end.value_or(scanner.line_end(start));
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
