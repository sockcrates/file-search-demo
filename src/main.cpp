#include "cli/arguments.h"
#include "file_io/file_reader.h"
#include "search/lower_bound.h"

#include <exception>
#include <iostream>

int main(int argc, char **argv) {
  const auto parsed = find::cli::parse_arguments(argc, argv);
  if (!parsed) {
    std::cerr << parsed.error() << '\n';
    return 2;
  }

  try {
    find::file_io::FileReader file(parsed->filename);
    const auto result = find::search::lower_bound(file, parsed->term);
    if (result) {
      std::cout << *result << '\n';
      return 0;
    }
    return 1;
  } catch (const std::exception &error) {
    std::cerr << "find: " << error.what() << '\n';
    return 2;
  }
}
