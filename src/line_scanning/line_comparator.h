#pragma once

#include "file_io/reader.h"

#include <compare>
#include <string_view>
#include <vector>

namespace find::line_scanning {

class LineComparator {
public:
  explicit LineComparator(const file_io::Reader &reader, std::size_t buffer_size = 64U * 1024U);
  [[nodiscard]] std::strong_ordering compare(std::uint64_t start, std::uint64_t end,
                                             std::string_view term) const;
  [[nodiscard]] std::string read_line(std::uint64_t start, std::uint64_t end) const;

private:
  const file_io::Reader &reader_;
  mutable std::vector<std::byte> buffer_;
};

} // namespace
  // find::line_scanning
