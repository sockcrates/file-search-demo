#include "file_io/file_reader.h"
#include "file_io/memory_reader.h"
#include "test_support.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>

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
  std::filesystem::remove(path);
}
