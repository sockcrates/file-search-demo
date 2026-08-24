#pragma once

#include "file_io/contiguous_reader.h"

#include <span>
#include <string_view>
#include <vector>

namespace find::file_io {

/** @brief In-memory implementation of the random-access Reader interface. */
class MemoryReader final : public ContiguousReader {
public:
  /**
   * @brief Copy @p bytes into a read-only random-access buffer.
   * @param bytes Source bytes copied into this reader.
   */
  explicit MemoryReader(std::span<const std::byte> bytes);
  /**
   * @brief Copy the explicitly sized text bytes into the buffer.
   * @param text Source text bytes copied into this reader.
   *
   * Unlike a C-string interface, this constructor is binary-safe and cannot
   * dereference a null pointer while determining the input length.
   */
  explicit MemoryReader(std::string_view text);
  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc ContiguousReader::bytes */
  [[nodiscard]] std::span<const std::byte> bytes() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::span<std::byte> destination) const override;

private:
  std::vector<std::byte> bytes_;
};

} // namespace
  // find::file_io
