#pragma once

#include "file_io/reader.h"

#include <cstdint>
#include <vector>

namespace find::line_scanning {

/** @brief Half-open byte range occupied by a line, excluding its newline. */
struct LineRange {
  /** @brief Inclusive byte offset of the first character in the line. */
  std::uint64_t start;
  /** @brief Exclusive byte offset of the line, before its terminating newline. */
  std::uint64_t end; // Excludes the terminating newline.
};

/**
 * @brief Locates newline-delimited line boundaries without loading a whole file.
 *
 * An unterminated final line is valid. Consecutive newlines form empty lines.
 * Only line-feed (`'\\n'`) is recognized; CRLF input is not normalized.
 */
class LineScanner {
public:
  /**
   * @brief Create a scanner over @p reader using a reusable chunk buffer.
   * @param buffer_size Size, in bytes, of the read buffer; must be non-zero.
   * @throws std::invalid_argument If @p buffer_size is zero.
   */
  explicit LineScanner(const file_io::Reader &reader, std::size_t buffer_size = 4U * 1024U);

  /**
   * @brief Find the first byte of the line containing @p offset.
   * @param offset Byte position, clamped to the end of the reader when larger.
   *               A newline position belongs to the preceding line.
   * @return The line's starting byte offset, or zero for an empty reader.
   */
  [[nodiscard]] std::uint64_t line_start_containing(std::uint64_t offset) const;

  /**
   * @brief Find the exclusive end of the line starting at @p start.
   * @return The newline offset, or the reader size for an unterminated final line.
   */
  [[nodiscard]] std::uint64_t line_end(std::uint64_t start) const;

  /** @brief Return the newline-excluding range of the line containing @p offset. */
  [[nodiscard]] LineRange line_containing(std::uint64_t offset) const;

private:
  const file_io::Reader &reader_;
  mutable std::vector<std::byte> buffer_;
};

} // namespace
  // find::line_scanning
