#pragma once

#include <cstddef>
#include <cstdint>

namespace find::file_io {

/**
 * @brief Random-access, read-only byte source.
 *
 * Implementations return data beginning at an absolute byte offset. They must
 * not write more than @p capacity bytes to the supplied destination.
 */
class Reader {
public:
  /** @brief Destroy the reader through its interface. */
  virtual ~Reader() = default;

  /** @brief Return the total number of bytes available from the reader. */
  [[nodiscard]] virtual std::uint64_t size() const = 0;

  /**
   * @brief Read a contiguous range of bytes.
   *
   * @param offset Zero-based byte offset at which to start reading.
   * @param destination Buffer that receives the bytes read.
   * @param capacity Maximum number of bytes to write to @p destination.
   * @return Number of bytes read, which is zero at or past end-of-input or when
   *         @p capacity is zero.
   */
  virtual std::size_t read(std::uint64_t offset, std::byte *destination,
                           std::size_t capacity) const = 0;
};

} // namespace
  // find::file_io
