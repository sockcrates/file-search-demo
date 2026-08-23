#pragma once

#include "file_io/reader.h"

#include <span>
#include <vector>

namespace find::file_io {

/** @brief In-memory implementation of the random-access Reader interface. */
class MemoryReader final : public Reader {
public:
  /** @brief Copy @p bytes into a read-only random-access buffer. */
  explicit MemoryReader(std::span<const std::byte> bytes);
  /** @brief Copy the null-terminated string at @p text into the buffer. */
  explicit MemoryReader(const char *text);
  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override;

private:
  std::vector<std::byte> bytes_;
};

} // namespace
  // find::file_io
