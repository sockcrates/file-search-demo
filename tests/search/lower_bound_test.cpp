#include "file_io/memory_reader.h"
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
