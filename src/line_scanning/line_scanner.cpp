#include "line_scanning/line_scanner.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace find::line_scanning {
namespace {
constexpr std::byte newline{static_cast<unsigned char>('\n')};
constexpr std::size_t probe_buffer_size = 512;
} // namespace

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
    const auto requested = static_cast<std::size_t>(position - chunk_start);
    std::size_t count = 0;
    while (count < requested) {
      const auto read_count = reader_.read(
          chunk_start + count, std::span{buffer_}.subspan(count, requested - count));
      if (read_count == 0)
        break;
      count += read_count;
    }
    for (std::size_t index = count; index > 0; --index) {
      if (buffer_[index - 1] == newline)
        return chunk_start + index;
    }
    if (count < requested)
      break;
    position = chunk_start;
  }
  return 0;
}

std::uint64_t LineScanner::line_end(std::uint64_t start) const {
  const auto file_size = reader_.size();
  auto position = std::min(start, file_size);
  while (position < file_size) {
    const auto count = reader_.read(position, std::span{buffer_});
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

std::optional<LineProbe> LineScanner::probe_containing(std::uint64_t offset,
                                                       std::string_view term) const {
  const auto file_size = reader_.size();
  if (file_size == 0)
    return std::nullopt;

  const auto anchor = std::min(offset, file_size);
  const auto window_size = std::min<std::uint64_t>({buffer_.size(), probe_buffer_size, file_size});
  const auto half_window = window_size / 2;
  const auto desired_start = anchor > half_window ? anchor - half_window : 0;
  const auto window_start = std::min(desired_start, file_size - window_size);
  const auto count =
      reader_.read(window_start, std::span{buffer_}.first(static_cast<std::size_t>(window_size)));
  if (count == 0)
    return std::nullopt;

  const auto anchor_index = static_cast<std::size_t>(anchor - window_start);
  if (anchor_index > count)
    return std::nullopt;
  std::size_t start_index = anchor_index;
  while (start_index > 0 && buffer_[start_index - 1] != newline)
    --start_index;
  if (start_index == 0 && window_start != 0)
    return std::nullopt;

  const auto *end_position = static_cast<const std::byte *>(
      std::memchr(buffer_.data() + start_index, static_cast<int>('\n'), count - start_index));
  std::size_t end_index = count;
  if (end_position != nullptr)
    end_index = static_cast<std::size_t>(end_position - buffer_.data());
  else if (window_start + count != file_size)
    return std::nullopt;

  const auto line_size = end_index - start_index;
  const auto common_size = std::min(line_size, term.size());
  const auto comparison = std::memcmp(buffer_.data() + start_index, term.data(), common_size);
  const auto ordering = comparison < 0            ? std::strong_ordering::less
                        : comparison > 0          ? std::strong_ordering::greater
                        : line_size < term.size() ? std::strong_ordering::less
                        : line_size > term.size() ? std::strong_ordering::greater
                                                  : std::strong_ordering::equal;
  return LineProbe{{window_start + start_index, window_start + end_index}, ordering};
}

} // namespace
  // find::line_scanning
