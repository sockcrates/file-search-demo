#pragma once

#include "file_io/reader.h"

#include <span>

namespace find::file_io {

/** @brief Reader whose complete contents are available as a stable byte span. */
class ContiguousReader : public Reader {
public:
  /** @brief Return all bytes in this reader without copying them. */
  [[nodiscard]] virtual std::span<const std::byte> bytes() const = 0;
};

} // namespace find::file_io
