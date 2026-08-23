#include "cli/arguments.h"

namespace find::cli {

std::expected<Arguments, UsageError> parse_arguments(std::span<const std::string_view> arguments) {
  constexpr std::size_t required_argument_count = 3U;
  if (arguments.size() != required_argument_count) {
    return std::unexpected(UsageError{UsageErrorCode::incorrect_argument_count, arguments.size()});
  }
  return Arguments{std::string{arguments[1]}, std::filesystem::path{arguments[2]}};
}

} // namespace
  // find::cli
