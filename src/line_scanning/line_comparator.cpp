#include "line_scanning/line_comparator.h"

#include <algorithm>
#include <stdexcept>

namespace find::line_scanning {

LineComparator::LineComparator(const file_io::Reader &reader, std::size_t buffer_size)
    : reader_(reader), buffer_(buffer_size, '\0') {
  if (buffer_size == 0)
    throw std::invalid_argument("line comparator buffer must not be empty");
}

LineComparison LineComparator::compare_from(std::uint64_t start, std::uint64_t file_size,
                                            std::string_view term) const {
  auto offset = start;
  std::size_t term_offset = 0;
  while (offset < file_size) {
    const auto term_remaining = term.size() - term_offset;
    const auto capacity = std::min<std::uint64_t>(
        buffer_.size(),
        std::min(file_size - offset, static_cast<std::uint64_t>(term_remaining) + 1));
    const auto count =
        reader_.read(offset, std::span{buffer_}.first(static_cast<std::size_t>(capacity)));
    if (count == 0)
      throw std::runtime_error("unexpected end of file while comparing line");

    const auto chunk = std::string_view{buffer_.data(), count};
    const auto newline_position = chunk.find('\n');
    const auto content_size = newline_position == std::string_view::npos ? count : newline_position;
    const auto common_size = std::min(content_size, term_remaining);
    if (common_size != 0) {
      const auto comparison =
          chunk.substr(0, common_size) <=> term.substr(term_offset, common_size);
      if (comparison == std::strong_ordering::less)
        return {std::strong_ordering::less, std::nullopt};
      if (comparison == std::strong_ordering::greater)
        return {std::strong_ordering::greater, std::nullopt};
    }

    offset += common_size;
    term_offset += common_size;
    if (common_size < content_size)
      return {std::strong_ordering::greater, std::nullopt};
    if (newline_position != std::string_view::npos)
      return {term_offset == term.size() ? std::strong_ordering::equal : std::strong_ordering::less,
              offset};
    if (term_offset == term.size() && offset == file_size)
      return {std::strong_ordering::equal, std::nullopt};
  }
  return {term_offset == term.size() ? std::strong_ordering::equal : std::strong_ordering::less,
          std::nullopt};
}

std::string LineComparator::read_line(std::uint64_t start, std::uint64_t end) const {
  std::string result;
  result.reserve(static_cast<std::size_t>(end - start));
  for (auto offset = start; offset < end;) {
    const auto count =
        reader_.read(offset, std::span{buffer_}.first(static_cast<std::size_t>(
                                 std::min<std::uint64_t>(buffer_.size(), end - offset))));
    if (count == 0)
      throw std::runtime_error("unexpected end of file while reading line");
    result.append(buffer_.data(), count);
    offset += count;
  }
  return result;
}

} // namespace
  // find::line_scanning
