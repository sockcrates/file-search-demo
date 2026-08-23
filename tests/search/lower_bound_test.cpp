#include "file_io/memory_reader.h"
#include "file_io/reader.h"
#include "search/lower_bound.h"
#include "test_support.h"

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
  explicit CountingReader(const find::file_io::Reader &reader) : reader_(reader) {}

  [[nodiscard]] std::uint64_t size() const override { return reader_.size(); }
  std::size_t read(std::uint64_t offset, std::byte *destination,
                   std::size_t capacity) const override {
    ++read_calls;
    const auto count = reader_.read(offset, destination, capacity);
    bytes_read += count;
    return count;
  }

  mutable std::size_t read_calls = 0;
  mutable std::size_t bytes_read = 0;

private:
  const find::file_io::Reader &reader_;
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
  find::file_io::MemoryReader reader(input.c_str());
  REQUIRE(find::search::lower_bound(reader, std::string(99999, 'a') + "z") ==
          std::string(100000, 'b'));
}

TEST(lower_bound_handles_a_line_larger_than_ten_mebibytes) {
  const std::string long_line(10U * 1024U * 1024U + 1U, 'a');
  const std::string input = long_line + "\nb";
  find::file_io::MemoryReader reader(input.c_str());
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
  for (const auto &line : lines)
    input += line + '\n';
  for (const auto &term : {"", "a", "aa", "apple", "apz", "b", "z", "zz", "zzz"}) {
    std::optional<std::string> expected;
    for (const auto &line : lines) {
      if (line >= term) {
        expected = line;
        break;
      }
    }
    find::file_io::MemoryReader reader(input.c_str());
    REQUIRE(find::search::lower_bound(reader, term) == expected);
  }
}

TEST(lower_bound_reads_a_sublinear_number_of_chunks_for_many_short_lines) {
  constexpr std::size_t line_count = 100000;
  std::string input;
  input.reserve(line_count * 8);
  for (std::size_t index = 0; index < line_count; ++index) {
    const auto number = std::to_string(index);
    input += "a" + std::string(6 - number.size(), '0') + number + '\n';
  }
  find::file_io::MemoryReader backing(input.c_str());
  CountingReader reader(backing);
  REQUIRE(find::search::lower_bound(reader, "a099999") == "a099999");
  REQUIRE(reader.read_calls < 100);
  REQUIRE(reader.bytes_read < 2U * 1024U * 1024U);
}

TEST(lower_bound_reuses_a_centered_probe_for_a_short_midpoint_line) {
  constexpr std::size_t line_count = 100000;
  std::string input;
  input.reserve(line_count * 8);
  for (std::size_t index = 0; index < line_count; ++index) {
    const auto number = std::to_string(index);
    input += "a" + std::string(6 - number.size(), '0') + number + '\n';
  }
  find::file_io::MemoryReader backing(input.c_str());
  CountingReader reader(backing);

  REQUIRE(find::search::lower_bound(reader, "a050000") == "a050000");
  REQUIRE(reader.read_calls == 1);
}
