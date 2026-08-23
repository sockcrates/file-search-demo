#pragma once

#include <filesystem>
#include <stdexcept>
#include <system_error>

namespace find::file_io {

/** @brief POSIX file operation that failed while constructing or reading a FileReader. */
enum class FileOperation {
  /** @brief Opening the requested path failed. */
  open,
  /** @brief Inspecting the opened file descriptor failed. */
  inspect,
  /** @brief The opened path was not a regular file. */
  validate_regular_file,
  /** @brief The file size could not be represented safely in memory. */
  validate_size,
  /** @brief Creating the read-only memory mapping failed. */
  map,
  /** @brief Reading a byte range from the opened descriptor failed. */
  read,
};

/**
 * @brief Structured operational failure from FileReader.
 *
 * The retained operation, path, and system error code let callers make a
 * programmatic decision without parsing the diagnostic returned by what().
 */
class FileError final : public std::runtime_error {
public:
  /** @brief Construct an error describing @p operation on @p path. */
  FileError(FileOperation operation, std::filesystem::path path, std::error_code system_error);

  /** @brief Failed file operation. */
  [[nodiscard]] FileOperation operation() const noexcept;
  /** @brief Requested path associated with the failed operation. */
  [[nodiscard]] const std::filesystem::path &path() const noexcept;
  /** @brief POSIX or validation error code captured at the failure boundary. */
  [[nodiscard]] const std::error_code &system_error() const noexcept;

private:
  FileOperation operation_;
  std::filesystem::path path_;
  std::error_code system_error_;
};

} // namespace find::file_io
