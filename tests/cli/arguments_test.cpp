#include "cli/arguments.h"
#include "test_support.h"

#include <array>
#include <string_view>

TEST(cli_requires_exactly_two_arguments) {
  constexpr std::array valid{std::string_view{"find"}, std::string_view{"apple"},
                             std::string_view{"fruit.txt"}};
  constexpr std::array too_few{std::string_view{"find"}};
  REQUIRE(find::cli::parse_arguments(too_few) ==
          std::unexpected(find::cli::UsageError{find::cli::UsageErrorCode::incorrect_argument_count,
                                                too_few.size()}));
  const auto parsed = find::cli::parse_arguments(valid);
  REQUIRE(parsed.has_value());
  REQUIRE(parsed->term == "apple");
  REQUIRE(parsed->filename == "fruit.txt");

  constexpr std::array too_many{std::string_view{"find"}, std::string_view{"apple"},
                                std::string_view{"fruit.txt"}, std::string_view{"extra"}};
  REQUIRE(find::cli::parse_arguments(too_many) ==
          std::unexpected(find::cli::UsageError{find::cli::UsageErrorCode::incorrect_argument_count,
                                                too_many.size()}));
}

TEST(cli_arguments_own_their_values) {
  std::string term{"apple"};
  std::string filename{"fruit.txt"};
  const std::array values{std::string_view{"find"}, std::string_view{term},
                          std::string_view{filename}};
  const auto parsed = find::cli::parse_arguments(values);
  term = "pear";
  filename = "other.txt";
  REQUIRE(parsed.has_value());
  REQUIRE(parsed->term == "apple");
  REQUIRE(parsed->filename == "fruit.txt");
}
