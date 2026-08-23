#include "cli/arguments.h"
#include "test_support.h"

TEST(cli_requires_exactly_two_arguments) {
  char program[] = "find";
  char term[] = "apple";
  char filename[] = "fruit.txt";
  char *valid[] = {program, term, filename};
  REQUIRE(find::cli::parse_arguments(1, valid) ==
          std::unexpected(std::string("usage: find <search-term> <filename>")));
  const auto parsed = find::cli::parse_arguments(3, valid);
  REQUIRE(parsed.has_value());
  REQUIRE(parsed->term == "apple");
}
