#include "file_io/file_reader.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace find::file_io {
namespace {
std::runtime_error system_error(const std::string &action, const std::filesystem::path &path) {
  return std::runtime_error(action + " '" + path.string() + "': " + std::strerror(errno));
}
} // namespace

FileReader::FileReader(const std::filesystem::path &path) {
  descriptor_ = open(path.c_str(), O_RDONLY);
  if (descriptor_ == -1)
    throw system_error("cannot open", path);
  struct stat details{};
  if (fstat(descriptor_, &details) == -1) {
    const auto error = system_error("cannot inspect", path);
    close(descriptor_);
    descriptor_ = -1;
    throw error;
  }
  if (details.st_size < 0)
    throw std::runtime_error("file has a negative size");
  if (!S_ISREG(details.st_mode)) {
    close(descriptor_);
    descriptor_ = -1;
    throw std::runtime_error("file is not a regular file");
  }
  size_ = static_cast<std::uint64_t>(details.st_size);
  if (size_ > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    close(descriptor_);
    descriptor_ = -1;
    throw std::runtime_error("file size exceeds platform limit");
  }
  if (size_ != 0) {
    const auto mapped =
        mmap(nullptr, static_cast<std::size_t>(size_), PROT_READ, MAP_PRIVATE, descriptor_, 0);
    if (mapped == MAP_FAILED) {
      const auto error = system_error("cannot map", path);
      close(descriptor_);
      descriptor_ = -1;
      throw error;
    }
    mapping_ = static_cast<const std::byte *>(mapped);
  }
}

FileReader::~FileReader() {
  if (mapping_ != nullptr)
    munmap(const_cast<std::byte *>(mapping_), static_cast<std::size_t>(size_));
  if (descriptor_ != -1)
    close(descriptor_);
}

std::uint64_t FileReader::size() const { return size_; }

std::span<const std::byte> FileReader::bytes() const {
  return {mapping_, static_cast<std::size_t>(size_)};
}

std::size_t FileReader::read(std::uint64_t offset, std::byte *destination,
                             std::size_t capacity) const {
  if (offset >= size_ || capacity == 0)
    return 0;
  if (offset > static_cast<std::uint64_t>(std::numeric_limits<off_t>::max())) {
    throw std::runtime_error("file offset exceeds platform limit");
  }
  const auto allowed = std::min<std::uint64_t>(capacity, size_ - offset);
  const auto requested = static_cast<std::size_t>(allowed);
  const auto count = pread(descriptor_, destination, requested, static_cast<off_t>(offset));
  if (count < 0)
    throw std::runtime_error(std::string("cannot read file: ") + std::strerror(errno));
  return static_cast<std::size_t>(count);
}

} // namespace
  // find::file_io
