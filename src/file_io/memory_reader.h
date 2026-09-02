#ifndef FIND_FILE_IO_MEMORY_READER_H
#define FIND_FILE_IO_MEMORY_READER_H

#include "file_io/reader.h"

#include <string>
#include <string_view>

namespace find::file_io {

/** @brief In-memory implementation of the random-access Reader interface. */
class MemoryReader final : public Reader {
public:
  /**
   * @brief Copy the explicitly sized text bytes into the buffer.
   * @param text Source text bytes copied into this reader.
   *
   * Unlike a C-string interface, this constructor is binary-safe and cannot
   * dereference a null pointer while determining the input length.
   */
  explicit MemoryReader(std::string_view text);
  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc Reader::bytes */
  [[nodiscard]] std::string_view bytes() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::span<char> destination) const override;

private:
  std::string bytes_;
};

} // namespace
  // find::file_io

#endif // FIND_FILE_IO_MEMORY_READER_H
