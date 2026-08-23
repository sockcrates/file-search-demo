#include "file_io/memory_reader.h"

#include <algorithm>
#include <cstring>
#include <limits>

namespace find::file_io {

MemoryReader::MemoryReader(std::span<const std::byte> bytes) : bytes_(bytes.begin(), bytes.end()) {}

MemoryReader::MemoryReader(const char *text) {
  const auto length = std::strlen(text);
  bytes_.resize(length);
  std::memcpy(bytes_.data(), text, length);
}

std::uint64_t MemoryReader::size() const { return static_cast<std::uint64_t>(bytes_.size()); }

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
