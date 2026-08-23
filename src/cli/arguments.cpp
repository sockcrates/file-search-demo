#include "cli/arguments.h"

namespace find::cli {

std::expected<Arguments, std::string> parse_arguments(int argc, char **argv) {
  if (argc != 3)
    return std::unexpected("usage: find <search-term> <filename>");
  return Arguments{argv[1], argv[2]};
}

} // namespace
  // find::cli
