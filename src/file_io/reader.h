#pragma once

#include <cstddef>
#include <cstdint>

namespace find::file_io {

class Reader {
public:
  virtual ~Reader() = default;
  [[nodiscard]] virtual std::uint64_t size() const = 0;
  virtual std::size_t read(std::uint64_t offset, std::byte *destination,
                           std::size_t capacity) const = 0;
};

} // namespace
  // find::file_io
