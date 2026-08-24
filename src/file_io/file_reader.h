#pragma once

#include "file_io/contiguous_reader.h"

#include <filesystem>
#include <memory>

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
   * @brief Open and map a stable regular file for read-only random access.
   * @param path Regular file to open and map.
   * @throws FileError If opening, inspecting, validating, mapping, or reading
   *         the file fails. The error identifies the operation, path, and code.
   */
  explicit FileReader(const std::filesystem::path &path);
  /** @brief Release the mapping and close the open file descriptor, if any. */
  ~FileReader() override;
  /** @brief Copying a resource-owning mapped reader is not supported. */
  FileReader(const FileReader &) = delete;
  /** @brief Copy assignment of a resource-owning mapped reader is not supported. */
  FileReader &operator=(const FileReader &) = delete;

  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc ContiguousReader::bytes */
  [[nodiscard]] std::span<const std::byte> bytes() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::span<std::byte> destination) const override;

private:
  class FileDescriptor;
  class Mapping;

  std::unique_ptr<FileDescriptor> descriptor_;
  std::filesystem::path path_;
  std::uint64_t size_ = 0;
  std::unique_ptr<Mapping> mapping_;
};

} // namespace
  // find::file_io
