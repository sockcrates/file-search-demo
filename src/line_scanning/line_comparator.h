#pragma once

#include "file_io/reader.h"

#include <compare>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace find::line_scanning {

/** @brief Result of comparing a line whose end may have been observed. */
struct LineComparison {
  std::strong_ordering ordering;
  std::optional<std::uint64_t> end;
};

/** @brief Compares and materializes line ranges using a bounded reusable buffer. */
class LineComparator {
public:
  /**
   * @brief Create a comparator over @p reader using a reusable chunk buffer.
   * @param buffer_size Size, in bytes, of the read buffer; must be non-zero.
   * @throws std::invalid_argument If @p buffer_size is zero.
   */
  explicit LineComparator(const file_io::Reader &reader, std::size_t buffer_size = 64U * 1024U);

  /**
   * @brief Compare a line range with a search term using unsigned ASCII bytes.
   * @param start Inclusive start of the line range.
   * @param end Exclusive end of the line range.
   * @param term Value to compare against the line.
   * @pre `start <= end <= reader.size()`.
   * @return The ordering of the line relative to @p term.
   */
  [[nodiscard]] std::strong_ordering compare(std::uint64_t start, std::uint64_t end,
                                             std::string_view term) const;

  /**
   * @brief Compare the line beginning at @p start without first locating its end.
   *
   * The comparison stops as soon as either a differing byte or the line's
   * newline is found. `end` is populated only when that newline was observed.
   */
  [[nodiscard]] LineComparison compare_from(std::uint64_t start, std::uint64_t file_size,
                                            std::string_view term) const;

  /**
   * @brief Copy a line range into an owning string.
   * @pre `start <= end <= reader.size()`.
   * @throws std::runtime_error If the reader ends before the requested range.
   */
  [[nodiscard]] std::string read_line(std::uint64_t start, std::uint64_t end) const;

private:
  const file_io::Reader &reader_;
  mutable std::vector<std::byte> buffer_;
};

} // namespace
  // find::line_scanning
