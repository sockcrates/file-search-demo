#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace find::cli {

struct Arguments {
  std::string_view term;
  std::filesystem::path filename;
};

[[nodiscard]] std::expected<Arguments, std::string> parse_arguments(int argc, char **argv);

} // namespace
  // find::cli
