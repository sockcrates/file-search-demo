#include "cli/arguments.h"
#include "file_io/file_reader.h"
#include "search/lower_bound.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>
#include <string_view>

namespace {

constexpr int no_match_exit_code = 1;
constexpr int failure_exit_code = 2;

} // namespace

int main(int argc, char **argv) {
  try {
    if (argc != 3) {
      std::cerr << find::cli::usage_message << '\n';
      return failure_exit_code;
    }
    const std::span supplied{argv, 3U};
    std::array<std::string_view, 3> arguments{};
    std::ranges::transform(supplied, arguments.begin(), [](const char *argument) noexcept {
      return std::string_view{argument};
    });
    const auto parsed = find::cli::parse_arguments(arguments);
    if (!parsed) {
      std::cerr << find::cli::usage_message << '\n';
      return failure_exit_code;
    }

    find::file_io::FileReader file(parsed->filename);
    const auto result = find::search::lower_bound(file, parsed->term);
    if (result) {
      std::cout << *result << '\n';
      return EXIT_SUCCESS;
    }
    return no_match_exit_code;
  } catch (const std::exception &error) {
    std::cerr << "find: " << error.what() << '\n';
    return failure_exit_code;
  }
}
