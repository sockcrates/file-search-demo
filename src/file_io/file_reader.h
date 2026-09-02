#ifndef FIND_FILE_IO_FILE_READER_H
#define FIND_FILE_IO_FILE_READER_H

#include "file_io/reader.h"

#include <filesystem>
#include <memory>

namespace find::file_io {

/**
 * @brief POSIX-backed random-access reader for a stable regular file on disk.
 *
 * Smaller files are memory-mapped for the reader's lifetime; larger files are
 * read with positioned I/O. Concurrent truncation of the file is unsupported.
 */
class FileReader final : public Reader {
public:
  /**
   * @brief Open a stable regular file for read-only random access.
   * @param path Regular file to open.
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
  /** @brief Move a resource-owning mapped reader. */
  FileReader(FileReader &&) noexcept;
  /** @brief Transfer ownership of a mapped reader's resources. */
  FileReader &operator=(FileReader &&) noexcept;

  /** @copydoc Reader::size */
  [[nodiscard]] std::uint64_t size() const override;
  /** @copydoc Reader::bytes
   *  @return The mapped contents, or an empty view when the file is too large
   *          to map as a whole.
   */
  [[nodiscard]] std::string_view bytes() const override;
  /** @copydoc Reader::read */
  std::size_t read(std::uint64_t offset, std::span<char> destination) const override;

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

#endif // FIND_FILE_IO_FILE_READER_H
