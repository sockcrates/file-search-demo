#pragma once

#include "file_io/reader.h"

#include <cstdint>
#include <vector>

namespace find::line_scanning {

struct LineRange {
  std::uint64_t start;
  std::uint64_t end; // Excludes the terminating newline.
};

class LineScanner {
public:
  explicit LineScanner(const file_io::Reader &reader, std::size_t buffer_size = 64U * 1024U);
  [[nodiscard]] std::uint64_t line_start_containing(std::uint64_t offset) const;
  [[nodiscard]] std::uint64_t line_end(std::uint64_t start) const;
  [[nodiscard]] LineRange line_containing(std::uint64_t offset) const;

private:
  const file_io::Reader &reader_;
  mutable std::vector<std::byte> buffer_;
};

} // namespace
  // find::line_scanning
