#include "file_io/file_reader.h"
#include "file_io/error.h"

#include <algorithm>
#include <cerrno>
#include <limits>
#include <system_error>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace find::file_io {
namespace {

constexpr std::uint64_t max_mapped_file_size = std::uint64_t{1} << 30U;

[[nodiscard]] std::error_code captured_errno() noexcept { return {errno, std::generic_category()}; }

} // namespace

class FileReader::FileDescriptor final {
public:
  FileDescriptor() = default;
  ~FileDescriptor() noexcept {
    if (descriptor_ != -1) {
      static_cast<void>(close(descriptor_));
    }
  }

  FileDescriptor(const FileDescriptor &) = delete;
  FileDescriptor &operator=(const FileDescriptor &) = delete;
  FileDescriptor(FileDescriptor &&) = delete;
  FileDescriptor &operator=(FileDescriptor &&) = delete;

  [[nodiscard]] int get() const noexcept { return descriptor_; }
  void reset(int descriptor = -1) noexcept {
    if (descriptor_ != -1) {
      static_cast<void>(close(descriptor_));
    }
    descriptor_ = descriptor;
  }

private:
  int descriptor_ = -1;
};

class FileReader::Mapping final {
public:
  Mapping() = default;
  ~Mapping() noexcept {
    if (address_ != nullptr) {
      static_cast<void>(munmap(address_, size_));
    }
  }

  Mapping(const Mapping &) = delete;
  Mapping &operator=(const Mapping &) = delete;
  Mapping(Mapping &&) = delete;
  Mapping &operator=(Mapping &&) = delete;

  [[nodiscard]] const char *data() const noexcept { return static_cast<const char *>(address_); }
  void reset(void *address, std::size_t size) noexcept {
    if (address_ != nullptr) {
      static_cast<void>(munmap(address_, size_));
    }
    address_ = address;
    size_ = size;
  }

private:
  void *address_ = nullptr;
  std::size_t size_ = 0;
};

FileReader::FileReader(const std::filesystem::path &path)
    : descriptor_(std::make_unique<FileDescriptor>()), path_(path),
      mapping_(std::make_unique<Mapping>()) {
  constexpr int flags = O_RDONLY | O_CLOEXEC | O_NONBLOCK;
  const auto opened_descriptor = open(path.c_str(), flags);
  if (opened_descriptor == -1) {
    throw FileError(FileOperation::open, path, captured_errno());
  }
  descriptor_->reset(opened_descriptor);

  struct stat details{};
  if (fstat(descriptor_->get(), &details) == -1) {
    throw FileError(FileOperation::inspect, path, captured_errno());
  }
  if (details.st_size < 0) {
    throw FileError(FileOperation::validate_size, path,
                    std::make_error_code(std::errc::value_too_large));
  }
  if (!S_ISREG(details.st_mode)) {
    throw FileError(FileOperation::validate_regular_file, path,
                    std::make_error_code(std::errc::invalid_argument));
  }
  const auto size = static_cast<std::uint64_t>(details.st_size);
  if (size != 0U && size <= max_mapped_file_size &&
      size <= static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    const auto mapped = mmap(nullptr, static_cast<std::size_t>(size), PROT_READ, MAP_PRIVATE,
                             descriptor_->get(), 0);
    if (mapped == MAP_FAILED) {
      throw FileError(FileOperation::map, path, captured_errno());
    }
    mapping_->reset(mapped, static_cast<std::size_t>(size));
    descriptor_->reset();
  }

  size_ = size;
}

FileReader::~FileReader() = default;

FileReader::FileReader(FileReader &&) noexcept = default;

FileReader &FileReader::operator=(FileReader &&) noexcept = default;

std::uint64_t FileReader::size() const { return size_; }

std::string_view FileReader::bytes() const {
  if (mapping_->data() == nullptr) {
    return {};
  }
  return {mapping_->data(), static_cast<std::size_t>(size_)};
}

std::size_t FileReader::read(std::uint64_t offset, std::span<char> destination) const {
  if (offset >= size_ || destination.empty())
    return 0;
  if (offset > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
    throw FileError(FileOperation::read, path_, std::make_error_code(std::errc::value_too_large));
  }
  const auto allowed = std::min<std::uint64_t>(destination.size(), size_ - offset);
  const auto requested = static_cast<std::size_t>(allowed);
  if (mapping_->data() != nullptr) {
    std::copy_n(mapping_->data() + static_cast<std::size_t>(offset), requested, destination.data());
    return requested;
  }
  for (;;) {
    const auto count =
        pread(descriptor_->get(), destination.data(), requested, static_cast<off_t>(offset));
    if (count >= 0) {
      return static_cast<std::size_t>(count);
    }
    if (errno != EINTR) {
      throw FileError(FileOperation::read, path_, captured_errno());
    }
  }
}

} // namespace
  // find::file_io
