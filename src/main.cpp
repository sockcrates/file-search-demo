#include "cli/arguments.h"
#include "file_io/file_reader.h"
#include "search/lower_bound.h"

#include <cstdint>
#include <exception>
#include <expected>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

namespace {

/** @brief Process exit statuses defined by the command-line interface. */
enum class ExitCode : std::uint8_t {
  /** @brief A lower-bound line was printed. */
  success = 0,
  /** @brief No line is equal to or greater than the term. */
  no_match = 1,
  /** @brief Invocation or file access failed. */
  failure = 2,
};

/** @brief Convert the C ABI command-line array into checked string views. */
[[nodiscard]] std::expected<std::vector<std::string_view>, find::cli::UsageError>
command_line_views(int argc, char *const *argv) {
  if (argc < 0 || argv == nullptr) {
    return std::unexpected(find::cli::UsageError{find::cli::UsageErrorCode::null_argument, 0U});
  }

  const auto count = static_cast<std::size_t>(argc);
  const std::span<char *const> supplied{argv, count};
  std::vector<std::string_view> views;
  views.reserve(count);
  for (const auto *argument : supplied) {
    if (argument == nullptr) {
      return std::unexpected(
          find::cli::UsageError{find::cli::UsageErrorCode::null_argument, count});
    }
    views.emplace_back(argument);
  }
  return views;
}

} // namespace

int main(int argc, char **argv) {
  try {
    const auto arguments = command_line_views(argc, argv);
    if (!arguments) {
      std::cerr << find::cli::usage_message << '\n';
      return static_cast<int>(ExitCode::failure);
    }
    const auto parsed = find::cli::parse_arguments(*arguments);
    if (!parsed) {
      std::cerr << find::cli::usage_message << '\n';
      return static_cast<int>(ExitCode::failure);
    }

    find::file_io::FileReader file(parsed->filename);
    const auto result = find::search::lower_bound(file, parsed->term);
    if (result) {
      std::cout << *result << '\n';
      return static_cast<int>(ExitCode::success);
    }
    return static_cast<int>(ExitCode::no_match);
  } catch (const std::exception &error) {
    std::cerr << "find: " << error.what() << '\n';
    return static_cast<int>(ExitCode::failure);
  }
}
