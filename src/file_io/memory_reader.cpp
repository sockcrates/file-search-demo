#include "file_io/memory_reader.h"

#include <algorithm>
#include <limits>

namespace find::file_io {

MemoryReader::MemoryReader(std::span<const std::byte> bytes) : bytes_(bytes.begin(), bytes.end()) {}

MemoryReader::MemoryReader(std::string_view text)
    : MemoryReader(std::as_bytes(std::span{text.data(), text.size()})) {}

std::uint64_t MemoryReader::size() const { return static_cast<std::uint64_t>(bytes_.size()); }

std::span<const std::byte> MemoryReader::bytes() const { return bytes_; }

std::size_t MemoryReader::read(std::uint64_t offset, std::byte *destination,
                               std::size_t capacity) const {
  if (offset >= size() || capacity == 0)
    return 0;
  if (offset > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    return 0;
  }
  const auto start = static_cast<std::size_t>(offset);
  const auto count = std::min(capacity, bytes_.size() - start);
  std::memcpy(destination, bytes_.data() + start, count);
  return count;
}

} // namespace
  // find::file_io
