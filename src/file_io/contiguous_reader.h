#ifndef FIND_FILE_IO_CONTIGUOUS_READER_H
#define FIND_FILE_IO_CONTIGUOUS_READER_H

#include "file_io/reader.h"

#include <span>

namespace find::file_io {

/** @brief Reader whose complete contents are available as a stable byte span. */
class ContiguousReader : public Reader {
public:
  /**
   * @brief Return all bytes in this reader without copying them.
   * @return A view valid until this reader is destroyed.
   */
  [[nodiscard]] virtual std::span<const std::byte> bytes() const = 0;
};

} // namespace find::file_io

#endif // FIND_FILE_IO_CONTIGUOUS_READER_H
