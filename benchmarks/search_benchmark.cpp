#include "file_io/file_reader.h"
#include "file_io/memory_reader.h"
#include "file_io/reader.h"
#include "search/lower_bound.h"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <unistd.h>

namespace {
class CountingReader final : public find::file_io::Reader {
public:
  explicit CountingReader(const find::file_io::Reader &reader) : reader_(reader) {}
  [[nodiscard]] std::uint64_t size() const override { return reader_.size(); }
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override {
    ++calls;
    const auto count = reader_.read(offset, destination, capacity);
    bytes += count;
    return count;
  }
  mutable std::size_t calls = 0;
  mutable std::size_t bytes = 0;

private:
  const find::file_io::Reader &reader_;
};

void run(std::string_view label, const std::string &data, std::string_view term) {
  find::file_io::MemoryReader backing(data.c_str());
  CountingReader reader(backing);
  const auto started = std::chrono::steady_clock::now();
  const auto result = find::search::lower_bound(reader, term);
  const auto elapsed = std::chrono::steady_clock::now() - started;
  std::cout << label << ": "
            << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " us, "
            << reader.bytes << " bytes read, " << reader.calls << " reads, result=";
  if (!result) {
    std::cout << "<none>\n";
    return;
  }
  constexpr std::size_t preview_size = 64;
  std::cout << result->size() << " bytes (\"" << result->substr(0, preview_size)
            << (result->size() > preview_size ? "...\"" : "\"") << ")\n";
}

void run_mapped(std::string_view label, const std::string &data, std::string_view term) {
  const auto path = std::filesystem::temp_directory_path() /
                    ("find-benchmark-" + std::to_string(getpid()) + ".txt");
  if (std::filesystem::exists(path))
    throw std::runtime_error("mapped benchmark file already exists");
  {
    std::ofstream output(path, std::ios::binary);
    output.write(data.data(), static_cast<std::streamsize>(data.size()));
    if (!output)
      throw std::runtime_error("cannot write mapped benchmark file");
  }

  try {
    find::file_io::FileReader reader(path);
    const auto started = std::chrono::steady_clock::now();
    const auto result = find::search::lower_bound(reader, term);
    const auto elapsed = std::chrono::steady_clock::now() - started;
    std::cout << label << ": "
              << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
              << " us, direct mapped search, result=";
    if (!result) {
      std::cout << "<none>\n";
    } else {
      constexpr std::size_t preview_size = 64;
      std::cout << result->size() << " bytes (\"" << result->substr(0, preview_size)
                << (result->size() > preview_size ? "...\"" : "\"") << "\n";
    }
  } catch (...) {
    std::filesystem::remove(path);
    throw;
  }
  std::filesystem::remove(path);
}

std::string padded_key(std::string_view prefix, std::size_t index) {
  const auto number = std::to_string(index);
  return std::string(prefix) + std::string(6 - number.size(), '0') + number;
}
} // namespace

int main() {
  std::string short_lines;
  for (std::size_t index = 0; index < 500000; ++index) {
    short_lines += padded_key("key", index) + '\n';
  }
  run("many short lines (before first)", short_lines, "aaa");
  run("many short lines (near beginning)", short_lines, "key000001");
  run("many short lines (middle)", short_lines, "key250000");
  run("many short lines (near end)", short_lines, "key499999");
  run("many short lines (after last)", short_lines, "zzz");
  run_mapped("mapped many short lines (middle)", short_lines, "key250000");

  std::string duplicates;
  for (std::size_t index = 0; index < 300000; ++index)
    duplicates += "duplicate\n";
  run("many identical lines", duplicates, "duplicate");

  const std::string prefixes =
      std::string(4U * 1024U * 1024U, 'a') + "b\n" + std::string(4U * 1024U * 1024U, 'a') + "c\n";
  run("enormous common prefixes", prefixes, std::string(4U * 1024U * 1024U, 'a') + "b");
  run_mapped("mapped enormous common prefixes", prefixes,
             std::string(4U * 1024U * 1024U, 'a') + "b");

  const std::string enormous_lines = std::string(2U * 1024U * 1024U, 'a') + "\n" +
                                     std::string(2U * 1024U * 1024U, 'b') + "\n" +
                                     std::string(2U * 1024U * 1024U, 'c') + "\n";
  run("a few enormous lines", enormous_lines, "b");
}
