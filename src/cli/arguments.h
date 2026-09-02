#ifndef FIND_CLI_ARGUMENTS_H
#define FIND_CLI_ARGUMENTS_H

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace find::cli {

/**
 * @brief Validated command-line input for the `find` executable.
 *
 * Both members own their data, so this value remains valid after the command
 * line argument storage is released.
 */
struct Arguments {
  /** @brief Search term in the input file's unsigned-byte sort order. */
  std::string term;
  /** @brief Path to the sorted input file. */
  std::filesystem::path filename;
};

/** @brief Classifies a command-line parsing failure. */
enum class UsageErrorCode : std::uint8_t {
  /** @brief The invocation did not contain exactly a term and file path. */
  incorrect_argument_count,
};

/**
 * @brief Structured reason the command line cannot be used.
 *
 * This error is intentionally a value rather than preformatted text so that
 * callers can choose their own presentation while retaining its category.
 */
struct UsageError {
  /** @brief Specific invalid-command-line condition. */
  UsageErrorCode code = UsageErrorCode::incorrect_argument_count;
  /** @brief Number of entries supplied, including the executable name. */
  std::size_t argument_count = 0;

  /** @brief Compare usage failures by code and supplied argument count. */
  [[nodiscard]] friend bool operator==(const UsageError &, const UsageError &) = default;
};

/** @brief Stable usage text presented by the executable for a UsageError. */
inline constexpr std::string_view usage_message{"usage: find <search-term> <filename>"};

/**
 * @brief Parse the executable's required search term and filename arguments.
 *
 * @param arguments Command-line values, including the executable name.
 * @return Parsed arguments when exactly a search term and filename are supplied;
 *         otherwise, a documented UsageError.
 */
[[nodiscard]] std::expected<Arguments, UsageError>
parse_arguments(std::span<const std::string_view> arguments);

} // namespace
  // find::cli

#endif // FIND_CLI_ARGUMENTS_H
