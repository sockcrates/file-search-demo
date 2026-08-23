#include "file_io/error.h"

#include <string>

namespace find::file_io {
namespace {

[[nodiscard]] std::string operation_name(FileOperation operation) {
  switch (operation) {
  case FileOperation::open:
    return "open";
  case FileOperation::inspect:
    return "inspect";
  case FileOperation::validate_regular_file:
    return "validate regular file";
  case FileOperation::validate_size:
    return "validate size";
  case FileOperation::map:
    return "map";
  case FileOperation::read:
    return "read";
  }
  return "unknown operation";
}

[[nodiscard]] std::string make_message(FileOperation operation, const std::filesystem::path &path,
                                       const std::error_code &system_error) {
  return "cannot " + operation_name(operation) + " '" + path.string() +
         "': " + system_error.message();
}

} // namespace

FileError::FileError(FileOperation operation, std::filesystem::path path,
                     std::error_code system_error)
    : std::runtime_error(make_message(operation, path, system_error)), operation_(operation),
      path_(std::move(path)), system_error_(system_error) {}

FileOperation FileError::operation() const noexcept { return operation_; }

const std::filesystem::path &FileError::path() const noexcept { return path_; }

const std::error_code &FileError::system_error() const noexcept { return system_error_; }

} // namespace find::file_io
