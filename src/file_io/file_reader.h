#pragma once

#include "file_io/reader.h"

#include <filesystem>

namespace find::file_io {

class FileReader final : public Reader {
public:
  explicit FileReader(const std::filesystem::path &path);
  ~FileReader() override;
  FileReader(const FileReader &) = delete;
  FileReader &operator=(const FileReader &) = delete;
  [[nodiscard]] std::uint64_t size() const override;
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override;

private:
  int descriptor_ = -1;
  std::uint64_t size_ = 0;
};

} // namespace
  // find::file_io
