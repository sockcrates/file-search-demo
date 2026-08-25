#include "search/lower_bound.h"

#include "file_io/contiguous_reader.h"
#include "line_scanning/line_comparator.h"
#include "line_scanning/line_scanner.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace find::search {
namespace {

struct ContiguousComparison {
  std::strong_ordering ordering = std::strong_ordering::equal;
  std::optional<std::size_t> end;
};

ContiguousComparison compare_contiguous_line(const char *data, std::size_t size, std::size_t start,
                                             std::string_view term) {
  auto offset = start;
  std::size_t term_offset = 0;
  while (offset < size) {
    const auto term_remaining = term.size() - term_offset;
    const auto capacity = std::min(size - offset, term_remaining + 1);
    const auto *newline = static_cast<const char *>(std::memchr(data + offset, '\n', capacity));
    const auto content_size =
        newline == nullptr ? capacity : static_cast<std::size_t>(newline - (data + offset));
    const auto common_size = std::min(content_size, term_remaining);
    if (common_size != 0) {
      const auto comparison = std::memcmp(data + offset, term.data() + term_offset, common_size);
      if (comparison < 0)
        return {std::strong_ordering::less, std::nullopt};
      if (comparison > 0)
        return {std::strong_ordering::greater, std::nullopt};
    }
    offset += common_size;
    term_offset += common_size;
    if (common_size < content_size)
      return {std::strong_ordering::greater, std::nullopt};
    if (newline != nullptr)
      return {term_offset == term.size() ? std::strong_ordering::equal : std::strong_ordering::less,
              offset};
    if (term_offset == term.size() && offset == size)
      return {std::strong_ordering::equal, std::nullopt};
  }
  return {term_offset == term.size() ? std::strong_ordering::equal : std::strong_ordering::less,
          std::nullopt};
}

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
    const auto comparison = compare_contiguous_line(data, size, start, term);
    const auto ordering = comparison.ordering;
    if (ordering == std::strong_ordering::equal)
      return std::string(term);
    if (ordering == std::strong_ordering::less) {
      auto end = comparison.end;
      if (!end) {
        // Finding start already examined the prefix through midpoint. Resume at
        // midpoint so an early mismatch on a very long line is not scanned twice.
        // At a newline, this correctly returns midpoint as the current line's end.
        auto line_end = midpoint;
        while (line_end < size && data[line_end] != '\n')
          ++line_end;
        end = line_end;
      }
      low = *end < size ? *end + 1 : size;
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
  const auto file_size = reader.size();
  if (const auto *contiguous = dynamic_cast<const file_io::ContiguousReader *>(&reader)) {
    const auto bytes = contiguous->bytes();
    const auto size_matches =
        file_size <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max()) &&
        bytes.size() == static_cast<std::size_t>(file_size);
    if (size_matches)
      return lower_bound_contiguous(bytes, term);
  }
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
      // line_start_containing() has already read up to midpoint. When comparison
      // ends early, continue from midpoint to avoid revisiting that prefix. A
      // midpoint at a newline is itself the current line's end.
      const auto end =
          probe ? probe->range.end : comparison->end.value_or(scanner.line_end(midpoint));
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
