#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace find::cli {

/**
 * @brief Validated command-line input for the `find` executable.
 *
 * `term` is a non-owning view into the argument vector supplied to `main`.
 * The argument vector must therefore outlive this object.
 */
struct Arguments {
  /** @brief Search term view backed by the original argument vector. */
  std::string_view term;
  /** @brief Path to the sorted input file. */
  std::filesystem::path filename;
};

/**
 * @brief Parse the executable's required search term and filename arguments.
 *
 * @param argc Number of entries in @p argv.
 * @param argv Argument vector received by `main`.
 * @return Parsed arguments when exactly a search term and filename are supplied;
 *         otherwise, the command's usage message.
 */
[[nodiscard]] std::expected<Arguments, std::string> parse_arguments(int argc, char **argv);

} // namespace
  // find::cli
