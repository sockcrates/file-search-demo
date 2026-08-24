#include "test_support.h"

#include <iostream>

int main() {
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
