#include "file_io/contiguous_reader.h"
#include "file_io/file_reader.h"
#include "file_io/memory_reader.h"
#include "file_io/reader.h"
#include "search/lower_bound.h"
#include "test_support.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace {
std::optional<std::string> search(const char *input, std::string_view term) {
  find::file_io::MemoryReader reader(input);
  return find::search::lower_bound(reader, term);
}

class CountingReader final : public find::file_io::Reader {
public:
  explicit CountingReader(const find::file_io::Reader &reader) : reader_(&reader) {}

  [[nodiscard]] std::uint64_t size() const override { return reader_->size(); }
  std::size_t read(std::uint64_t offset, std::span<std::byte> destination) const override {
    ++read_calls_;
    const auto count = reader_->read(offset, destination);
    bytes_read_ += count;
    return count;
  }
  [[nodiscard]] std::size_t read_calls() const noexcept { return read_calls_; }
  [[nodiscard]] std::size_t bytes_read() const noexcept { return bytes_read_; }

private:
  const find::file_io::Reader *reader_;
  mutable std::size_t read_calls_ = 0;
  mutable std::size_t bytes_read_ = 0;
};

class ShortReadReader final : public find::file_io::Reader {
public:
  ShortReadReader(const find::file_io::Reader &reader, std::size_t maximum_read)
      : reader_(&reader), maximum_read_(maximum_read) {}

  [[nodiscard]] std::uint64_t size() const override { return reader_->size(); }
  std::size_t read(std::uint64_t offset, std::span<std::byte> destination) const override {
    return reader_->read(offset, destination.first(std::min(destination.size(), maximum_read_)));
  }

private:
  const find::file_io::Reader *reader_;
  std::size_t maximum_read_;
};

class NoReadContiguousReader final : public find::file_io::ContiguousReader {
public:
  explicit NoReadContiguousReader(const char *input) : backing_(input) {}

  [[nodiscard]] std::uint64_t size() const override { return backing_.size(); }
  [[nodiscard]] std::span<const std::byte> bytes() const override { return backing_.bytes(); }
  std::size_t read(std::uint64_t /*offset*/, std::span<std::byte> /*destination*/) const override {
    throw std::runtime_error("direct path must not read");
  }

private:
  find::file_io::MemoryReader backing_;
};
} // namespace

TEST(lower_bound_finds_expected_lines) {
  REQUIRE(search("apple\napricot\navocado\nbanana", "apri") == "apricot");
  REQUIRE(search("apple\napricot\navocado\nbanana", "apricot") == "apricot");
  REQUIRE(search("apple\napricot\navocado\nbanana", "a") == "apple");
  REQUIRE(search("apple\napricot\navocado\nbanana", "z") == std::nullopt);
}

TEST(lower_bound_handles_duplicates_empty_and_unterminated_lines) {
  REQUIRE(search("\n\napple\napple\nbanana", "") == "");
  REQUIRE(search("\n\napple\napple\nbanana", "apple") == "apple");
  REQUIRE(search("only", "only") == "only");
  REQUIRE(search("only", "z") == std::nullopt);
  REQUIRE(search("", "anything") == std::nullopt);
}

TEST(lower_bound_handles_long_lines) {
  const std::string input = std::string(100000, 'a') + "\n" + std::string(100000, 'b');
  find::file_io::MemoryReader reader(input);
  REQUIRE(find::search::lower_bound(reader, std::string(99999, 'a') + "z") ==
          std::string(100000, 'b'));
}

TEST(lower_bound_handles_short_reads_from_a_generic_reader) {
  find::file_io::MemoryReader backing("apple\napricot\navocado\nbanana");
  ShortReadReader reader(backing, 3U);
  REQUIRE(find::search::lower_bound(reader, "apri") == "apricot");
  REQUIRE(find::search::lower_bound(reader, "z") == std::nullopt);
}

TEST(lower_bound_handles_a_midpoint_at_a_line_terminator) {
  find::file_io::MemoryReader reader("a\nz");
  REQUIRE(find::search::lower_bound(reader, "m") == "z");
}

TEST(lower_bound_falls_back_for_a_large_line_with_a_generic_reader) {
  const std::string long_line((2U * 1024U) + 1U, 'a');
  find::file_io::MemoryReader backing(long_line + "\nb");
  CountingReader reader(backing);
  REQUIRE(find::search::lower_bound(reader, long_line + "z") == "b");
  REQUIRE(reader.read_calls() > 1U);
}

TEST(lower_bound_does_not_rescan_a_long_line_after_an_early_mismatch) {
  constexpr std::size_t mebibyte = std::size_t{1024U} * 1024U;
  constexpr std::size_t scan_slack = std::size_t{128U} * 1024U;
  const std::string long_line(8U * mebibyte, 'a');
  find::file_io::MemoryReader backing(long_line + "\nz");
  CountingReader reader(backing);

  REQUIRE(find::search::lower_bound(reader, "m") == "z");
  REQUIRE(reader.bytes_read() < long_line.size() + scan_slack);
}

TEST(lower_bound_handles_a_line_larger_than_ten_mebibytes) {
  constexpr std::size_t mebibyte = std::size_t{1024U} * 1024U;
  const std::string long_line((10U * mebibyte) + 1U, 'a');
  const std::string input = long_line + "\nb";
  find::file_io::MemoryReader reader(input);
  REQUIRE(find::search::lower_bound(reader, long_line + "z") == "b");
}

TEST(lower_bound_handles_files_made_only_of_newlines_and_repeated_searches) {
  find::file_io::MemoryReader reader("\n\n\n");
  for (int iteration = 0; iteration < 3; ++iteration) {
    REQUIRE(find::search::lower_bound(reader, "") == "");
    REQUIRE(find::search::lower_bound(reader, "a") == std::nullopt);
  }
}

TEST(lower_bound_matches_a_linear_reference_for_many_terms) {
  const std::vector<std::string> lines{"", "a", "apple", "apple", "apricot", "b", "zz"};
  std::string input;
  for (const auto &line : lines) {
    input += line + '\n';
  }
  for (const auto &term : {"", "a", "aa", "apple", "apz", "b", "z", "zz", "zzz"}) {
    std::optional<std::string> expected;
    for (const auto &line : lines) {
      if (line >= term) {
        expected = line;
        break;
      }
    }
    find::file_io::MemoryReader reader(input);
    REQUIRE(find::search::lower_bound(reader, term) == expected);
  }
}

TEST(lower_bound_reads_a_sublinear_number_of_chunks_for_many_short_lines) {
  constexpr std::size_t line_count = 100000;
  constexpr std::size_t characters_per_line = 8U;
  constexpr std::size_t key_width = 6U;
  constexpr std::size_t maximum_bytes_read = std::size_t{2U} * 1024U * 1024U;
  std::string input;
  input.reserve(line_count * characters_per_line);
  for (std::size_t index = 0; index < line_count; ++index) {
    const auto number = std::to_string(index);
    input += "a" + std::string(key_width - number.size(), '0') + number + '\n';
  }
  find::file_io::MemoryReader backing(input);
  CountingReader reader(backing);
  REQUIRE(find::search::lower_bound(reader, "a099999") == "a099999");
  REQUIRE(reader.read_calls() < 100U);
  REQUIRE(reader.bytes_read() < maximum_bytes_read);
}

TEST(lower_bound_reuses_a_centered_probe_for_a_short_midpoint_line) {
  constexpr std::size_t line_count = 100000;
  constexpr std::size_t characters_per_line = 8U;
  constexpr std::size_t key_width = 6U;
  std::string input;
  input.reserve(line_count * characters_per_line);
  for (std::size_t index = 0; index < line_count; ++index) {
    const auto number = std::to_string(index);
    input += "a" + std::string(key_width - number.size(), '0') + number + '\n';
  }
  find::file_io::MemoryReader backing(input);
  CountingReader reader(backing);

  REQUIRE(find::search::lower_bound(reader, "a050000") == "a050000");
  REQUIRE(reader.read_calls() == 1U);
}

TEST(lower_bound_uses_a_contiguous_reader_without_copy_reads) {
  NoReadContiguousReader reader("apple\napricot\navocado\nbanana");
  REQUIRE(find::search::lower_bound(reader, "apri") == "apricot");
}

TEST(lower_bound_contiguous_path_matches_generic_reader_at_boundaries) {
  find::file_io::MemoryReader direct("\n\napple\napple\nbanana");
  CountingReader generic(direct);
  for (const auto *const term : {"", "apple", "apz", "banana", "z"}) {
    REQUIRE(find::search::lower_bound(direct, term) == find::search::lower_bound(generic, term));
  }
}

TEST(lower_bound_searches_a_mapped_file) {
  const auto path = std::filesystem::temp_directory_path() / "find-mapped-search-test.txt";
  {
    std::ofstream output(path, std::ios::binary);
    output << "apple\napricot\navocado\nbanana";
  }
  find::file_io::FileReader reader(path);
  REQUIRE(find::search::lower_bound(reader, "apri") == "apricot");
  std::filesystem::remove(path);
}
