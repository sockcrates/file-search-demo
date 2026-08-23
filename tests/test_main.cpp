#include "test_support.h"

#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  if (argc == 3 && std::string(argv[1]) == "--integration") {
    const auto command = std::string("'") + argv[2] + "' apri '" + std::string(TEST_SOURCE_DIR) +
                         "/tests/fixtures/fruits.txt' > /tmp/find-output";
    if (std::system(command.c_str()) != 0)
      return 1;
    return std::system((std::string("test \"") + "$(cat /tmp/find-output)\" = apricot").c_str()) ==
                   0
               ? 0
               : 1;
  }
  int failures = 0;
  for (const auto &[name, body] : test::cases()) {
    try {
      body();
    } catch (const std::exception &error) {
      ++failures;
      std::cerr << "FAIL " << name << ": " << error.what() << '\n';
    }
  }
  std::cout << test::cases().size() - static_cast<std::size_t>(failures) << " tests passed\n";
  return failures == 0 ? 0 : 1;
}
