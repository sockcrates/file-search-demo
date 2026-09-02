#include "file_io/memory_reader.h"

#include <algorithm>
#include <limits>

namespace find::file_io {

MemoryReader::MemoryReader(std::string_view text) : bytes_(text) {}

std::uint64_t MemoryReader::size() const { return static_cast<std::uint64_t>(bytes_.size()); }

std::string_view MemoryReader::bytes() const { return bytes_; }

std::size_t MemoryReader::read(std::uint64_t offset, std::span<char> destination) const {
  if (offset >= size() || destination.empty())
    return 0;
  if (offset > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    return 0;
  }
  const auto start = static_cast<std::size_t>(offset);
  const auto count = std::min(destination.size(), bytes_.size() - start);
  bytes_.copy(destination.data(), count, start);
  return count;
}

} // namespace
  // find::file_io
