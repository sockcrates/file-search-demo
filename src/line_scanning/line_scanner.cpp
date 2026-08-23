#include "line_scanning/line_scanner.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace find::line_scanning {
namespace {
constexpr std::byte newline{static_cast<unsigned char>('\n')};
}

LineScanner::LineScanner(const file_io::Reader &reader, std::size_t buffer_size)
    : reader_(reader), buffer_(buffer_size) {
  if (buffer_size == 0)
    throw std::invalid_argument("line scanner buffer must not be empty");
}

std::uint64_t LineScanner::line_start_containing(std::uint64_t offset) const {
  const auto file_size = reader_.size();
  if (file_size == 0)
    return 0;
  auto position = std::min(offset, file_size);
  while (position > 0) {
    const auto chunk_start = position > buffer_.size() ? position - buffer_.size() : 0;
    const auto count =
        reader_.read(chunk_start, buffer_.data(), static_cast<std::size_t>(position - chunk_start));
    for (std::size_t index = count; index > 0; --index) {
      if (buffer_[index - 1] == newline)
        return chunk_start + index;
    }
    position = chunk_start;
  }
  return 0;
}

std::uint64_t LineScanner::line_end(std::uint64_t start) const {
  const auto file_size = reader_.size();
  auto position = std::min(start, file_size);
  while (position < file_size) {
    const auto count = reader_.read(position, buffer_.data(), buffer_.size());
    if (count == 0)
      break;
    const auto *newline_position =
        static_cast<const std::byte *>(std::memchr(buffer_.data(), static_cast<int>('\n'), count));
    if (newline_position != nullptr)
      return position + static_cast<std::uint64_t>(newline_position - buffer_.data());
    position += count;
  }
  return file_size;
}

LineRange LineScanner::line_containing(std::uint64_t offset) const {
  const auto start = line_start_containing(offset);
  return {start, line_end(start)};
}

} // namespace
  // find::line_scanning
