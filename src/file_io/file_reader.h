#pragma once

#include "file_io/contiguous_reader.h"

#include <filesystem>

namespace find::file_io {

/**
 * @brief POSIX-backed random-access reader for a stable regular file on disk.
 *
 * The file is memory-mapped for the reader's lifetime. Concurrent truncation
 * of the mapped file is unsupported.
 */
class FileReader final : public ContiguousReader {
public:
  /**
   * @brief Open @p path for read-only random access.
   * @throws std::runtime_error If the file cannot be opened or inspected.
   */
  explicit FileReader(const std::filesystem::path &path);
  /** @brief Close the open file descriptor, if any. */
  ~FileReader() override;
  FileReader(const FileReader &) = delete;
  FileReader &operator=(const FileReader &) = delete;

  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc ContiguousReader::bytes */
  [[nodiscard]] std::span<const std::byte> bytes() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override;

private:
  int descriptor_ = -1;
  std::uint64_t size_ = 0;
  const std::byte *mapping_ = nullptr;
};

} // namespace
  // find::file_io
