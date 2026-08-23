#pragma once

#include "file_io/reader.h"

#include <span>
#include <vector>

namespace find::file_io {

class MemoryReader final : public Reader {
public:
  explicit MemoryReader(std::span<const std::byte> bytes);
  explicit MemoryReader(const char *text);
  [[nodiscard]] std::uint64_t size() const override;
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override;

private:
  std::vector<std::byte> bytes_;
};

} // namespace
  // find::file_io
