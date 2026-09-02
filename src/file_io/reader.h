#ifndef FIND_FILE_IO_READER_H
#define FIND_FILE_IO_READER_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace find::file_io {

/**
 * @brief Random-access, read-only character source.
 *
 * Implementations return data beginning at an absolute byte offset. They must
 * not write beyond the supplied destination span.
 */
class Reader {
public:
  Reader() = default;
  Reader(const Reader &) = default;
  Reader(Reader &&) = default;
  Reader &operator=(const Reader &) = default;
  Reader &operator=(Reader &&) = default;

  /** @brief Destroy the reader through its interface. */
  virtual ~Reader() = default;

  /** @brief Return the total number of bytes available from the reader. */
  [[nodiscard]] virtual std::uint64_t size() const = 0;

  /**
   * @brief Return all contents without copying when contiguous storage is available.
   * @return A stable view valid until this reader is destroyed, or an empty view
   *         when the reader has no contiguous representation.
   */
  [[nodiscard]] virtual std::string_view bytes() const { return {}; }

  /**
   * @brief Read a contiguous range of bytes.
   *
   * @param offset Zero-based byte offset at which to start reading.
   * @param destination Buffer that receives the bytes read. Its size bounds
   *        the operation, preventing a nullable pointer from being separated
   *        from its capacity.
   * @return Number of bytes read, which is zero at or past end-of-input or when
   *         @p destination is empty.
   */
  virtual std::size_t read(std::uint64_t offset, std::span<char> destination) const = 0;
};

} // namespace
  // find::file_io

#endif // FIND_FILE_IO_READER_H
