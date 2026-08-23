#include "file_io/error.h"
#include "file_io/file_reader.h"
#include "file_io/memory_reader.h"
#include "test_support.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>

TEST(memory_reader_handles_boundaries) {
  find::file_io::MemoryReader reader("abcdef");
  std::array<std::byte, 4> bytes{};
  REQUIRE(reader.size() == 6);
  REQUIRE(reader.read(0, bytes.data(), bytes.size()) == 4);
  REQUIRE(std::to_integer<char>(bytes[0]) == 'a');
  REQUIRE(reader.read(4, bytes.data(), bytes.size()) == 2);
  REQUIRE(std::to_integer<char>(bytes[1]) == 'f');
  REQUIRE(reader.read(6, bytes.data(), bytes.size()) == 0);
  REQUIRE(reader.read(99, bytes.data(), bytes.size()) == 0);
  REQUIRE(reader.read(UINT64_MAX, bytes.data(), bytes.size()) == 0);
}

TEST(memory_reader_handles_empty_input) {
  find::file_io::MemoryReader reader("");
  std::array<std::byte, 1> byte{};
  REQUIRE(reader.size() == 0);
  REQUIRE(reader.read(0, byte.data(), 1) == 0);
}

TEST(memory_reader_exposes_a_contiguous_view) {
  find::file_io::MemoryReader reader("abcdef");
  const auto bytes = reader.bytes();
  REQUIRE(bytes.size() == reader.size());
  REQUIRE(std::to_integer<char>(bytes[0]) == 'a');
  REQUIRE(std::to_integer<char>(bytes[5]) == 'f');
}

TEST(file_reader_reads_binary_data_without_loading_the_file) {
  const auto path = std::filesystem::temp_directory_path() / "find-reader-test.txt";
  {
    std::ofstream output(path, std::ios::binary);
    output << "first\n\nlonger final line";
  }
  find::file_io::FileReader reader(path);
  std::array<std::byte, 32> bytes{};
  REQUIRE(reader.size() == 24);
  REQUIRE(reader.read(6, bytes.data(), 8) == 8);
  REQUIRE(std::to_integer<char>(bytes[0]) == '\n');
  REQUIRE(std::to_integer<char>(bytes[1]) == 'l');
  REQUIRE(reader.read(reader.size(), bytes.data(), bytes.size()) == 0);
  const auto mapped = reader.bytes();
  REQUIRE(mapped.size() == reader.size());
  REQUIRE(std::to_integer<char>(mapped[6]) == '\n');
  REQUIRE(std::to_integer<char>(mapped[7]) == 'l');
  std::filesystem::remove(path);
}

TEST(file_reader_exposes_an_empty_contiguous_view_for_empty_files) {
  const auto path = std::filesystem::temp_directory_path() / "find-empty-reader-test.txt";
  std::ofstream(path, std::ios::binary);
  find::file_io::FileReader reader(path);
  REQUIRE(reader.size() == 0);
  REQUIRE(reader.bytes().empty());
  std::filesystem::remove(path);
}

TEST(file_reader_maps_binary_bytes_and_remains_valid_after_unlink) {
  const auto path = std::filesystem::temp_directory_path() / "find-binary-reader-test.txt";
  {
    std::ofstream output(path, std::ios::binary);
    const std::array<char, 4> contents{'a', '\0', 'b', '\n'};
    output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
  }
  find::file_io::FileReader reader(path);
  std::filesystem::remove(path);
  const auto mapped = reader.bytes();
  REQUIRE(mapped.size() == 4);
  REQUIRE(std::to_integer<char>(mapped[0]) == 'a');
  REQUIRE(std::to_integer<char>(mapped[1]) == '\0');
  REQUIRE(std::to_integer<char>(mapped[2]) == 'b');
  REQUIRE(std::to_integer<char>(mapped[3]) == '\n');
}

TEST(file_reader_reports_an_open_failure) {
  const auto missing = std::filesystem::temp_directory_path() / "find-no-such-file.txt";
  std::filesystem::remove(missing);
  bool threw = false;
  try {
    [[maybe_unused]] find::file_io::FileReader reader(missing);
  } catch (const find::file_io::FileError &error) {
    threw = error.operation() == find::file_io::FileOperation::open && error.path() == missing &&
            error.system_error().value() != 0;
  }
  REQUIRE(threw);
}

TEST(file_reader_rejects_a_directory) {
  bool threw = false;
  try {
    [[maybe_unused]] find::file_io::FileReader reader(std::filesystem::temp_directory_path());
  } catch (const find::file_io::FileError &error) {
    threw = error.operation() == find::file_io::FileOperation::validate_regular_file &&
            error.path() == std::filesystem::temp_directory_path();
  }
  REQUIRE(threw);
}
