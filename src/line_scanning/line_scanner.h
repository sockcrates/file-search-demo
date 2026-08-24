#ifndef FIND_LINE_SCANNING_LINE_SCANNER_H
#define FIND_LINE_SCANNING_LINE_SCANNER_H

#include "file_io/reader.h"

#include <compare>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace find::line_scanning {

inline constexpr std::size_t default_scanner_buffer_size = 4096U;

/** @brief Half-open byte range occupied by a line, excluding its newline. */
struct LineRange {
  /** @brief Inclusive byte offset of the first character in the line. */
  std::uint64_t start = 0;
  /** @brief Exclusive byte offset of the line, before its terminating newline. */
  std::uint64_t end = 0; // Excludes the terminating newline.
};

/** @brief A complete line and its ordering, determined from one centered read. */
struct LineProbe {
  /** @brief Newline-excluding byte range of the probed line. */
  LineRange range{};
  /** @brief Unsigned-byte ordering of `range` relative to the search term. */
  std::strong_ordering ordering = std::strong_ordering::equal;
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
   * @param reader Borrowed random-access source that must outlive this scanner.
   * @param buffer_size Size, in bytes, of the read buffer; must be non-zero.
   * @throws std::invalid_argument If @p buffer_size is zero.
   */
  explicit LineScanner(const file_io::Reader &reader,
                       std::size_t buffer_size = default_scanner_buffer_size);

  /**
   * @brief Find the first byte of the line containing @p offset.
   * @param offset Byte position, clamped to the end of the reader when larger.
   *               A newline position belongs to the preceding line.
   * @return The line's starting byte offset, or zero for an empty reader.
   */
  [[nodiscard]] std::uint64_t line_start_containing(std::uint64_t offset) const;

  /**
   * @brief Find the exclusive end of the line starting at @p start.
   * @param start Start offset of a line in the reader.
   * @pre `start <= reader.size()` and @p start begins a line.
   * @return The newline offset, or the reader size for an unterminated final line.
   */
  [[nodiscard]] std::uint64_t line_end(std::uint64_t start) const;

  /**
   * @brief Return the newline-excluding range of the line containing @p offset.
   * @param offset Byte position, clamped to the reader's end when larger.
   * @return The containing line's half-open range, or `{0, 0}` for an empty reader.
   */
  [[nodiscard]] LineRange line_containing(std::uint64_t offset) const;

  /**
   * @brief Compare the line around @p offset when it fits in one centered buffer.
   * @param offset Byte position whose line is probed.
   * @param term Search term compared using unsigned ASCII bytes.
   *
   * Returns `std::nullopt` when either line boundary lies outside the centered
   * buffer, so callers can fall back to directional chunk scanning. A newline
   * position belongs to the preceding line, matching line_start_containing().
   * @return The line range and its ordering, or `std::nullopt` when the line does not fit.
   */
  [[nodiscard]] std::optional<LineProbe> probe_containing(std::uint64_t offset,
                                                          std::string_view term) const;

private:
  const file_io::Reader &reader_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members):
                                  // non-null borrowed reader
  mutable std::vector<std::byte> buffer_;
};

} // namespace
  // find::line_scanning

#endif // FIND_LINE_SCANNING_LINE_SCANNER_H
