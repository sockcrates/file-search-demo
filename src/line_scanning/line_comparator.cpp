#include "line_scanning/line_comparator.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace find::line_scanning {

LineComparator::LineComparator(const file_io::Reader &reader, std::size_t buffer_size)
    : reader_(reader), buffer_(buffer_size) {
  if (buffer_size == 0)
    throw std::invalid_argument("line comparator buffer must not be empty");
}

std::strong_ordering LineComparator::compare(std::uint64_t start, std::uint64_t end,
                                             std::string_view term) const {
  auto offset = start;
  std::size_t term_offset = 0;
  while (offset < end && term_offset < term.size()) {
    const auto count = reader_.read(
        offset, buffer_.data(),
        std::min<std::uint64_t>(
            buffer_.size(),
            std::min(end - offset, static_cast<std::uint64_t>(term.size() - term_offset))));
    const auto comparison = std::memcmp(buffer_.data(), term.data() + term_offset, count);
    if (comparison < 0)
      return std::strong_ordering::less;
    if (comparison > 0)
      return std::strong_ordering::greater;
    offset += count;
    term_offset += count;
  }
  if (offset < end)
    return std::strong_ordering::greater;
  if (term_offset < term.size())
    return std::strong_ordering::less;
  return std::strong_ordering::equal;
}

std::string LineComparator::read_line(std::uint64_t start, std::uint64_t end) const {
  std::string result;
  result.reserve(static_cast<std::size_t>(end - start));
  for (auto offset = start; offset < end;) {
    const auto count =
        reader_.read(offset, buffer_.data(), std::min<std::uint64_t>(buffer_.size(), end - offset));
    if (count == 0)
      throw std::runtime_error("unexpected end of file while reading line");
    result.append(reinterpret_cast<const char *>(buffer_.data()), count);
    offset += count;
  }
  return result;
}

} // namespace
  // find::line_scanning
