#ifndef FIND_LINE_SCANNING_LINE_COMPARATOR_H
#define FIND_LINE_SCANNING_LINE_COMPARATOR_H

#include "file_io/reader.h"

#include <compare>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace find::line_scanning {

inline constexpr std::size_t default_comparator_buffer_size = 65536U;

/** @brief Result of comparing a line whose end may have been observed. */
struct LineComparison {
  /** @brief Unsigned-byte ordering of the observed line prefix relative to the search term. */
  std::strong_ordering ordering = std::strong_ordering::equal;
  /** @brief Newline offset when the complete line ended in the scanned data. */
  std::optional<std::uint64_t> end;
};

/** @brief Compares and materializes line ranges using a bounded reusable buffer. */
class LineComparator {
public:
  /**
   * @brief Create a comparator over @p reader using a reusable chunk buffer.
   * @param reader Borrowed random-access source that must outlive this comparator.
   * @param buffer_size Size, in bytes, of the read buffer; must be non-zero.
   * @throws std::invalid_argument If @p buffer_size is zero.
   */
  explicit LineComparator(const file_io::Reader &reader,
                          std::size_t buffer_size = default_comparator_buffer_size);

  /**
   * @brief Compare the line beginning at @p start without first locating its end.
   * @param start Inclusive start offset of the line.
   * @param file_size Total size of the reader being searched.
   * @param term Search term compared using unsigned ASCII bytes.
   *
   * The comparison stops as soon as either a differing byte or the line's
   * newline is found. `end` is populated only when that newline was observed.
   * @return The ordering and, when observed, the terminating newline's offset.
   */
  [[nodiscard]] LineComparison compare_from(std::uint64_t start, std::uint64_t file_size,
                                            std::string_view term) const;

  /**
   * @brief Copy a line range into an owning string.
   * @param start Inclusive start of the line range.
   * @param end Exclusive end of the line range.
   * @pre `start <= end <= reader.size()`.
   * @throws std::runtime_error If the reader ends before the requested range.
   * @return A string containing the requested bytes.
   */
  [[nodiscard]] std::string read_line(std::uint64_t start, std::uint64_t end) const;

private:
  const file_io::Reader &reader_; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members):
                                  // non-null borrowed reader
  mutable std::string buffer_;
};

} // namespace
  // find::line_scanning

#endif // FIND_LINE_SCANNING_LINE_COMPARATOR_H
