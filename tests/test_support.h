#ifndef FIND_TEST_SUPPORT_H
#define FIND_TEST_SUPPORT_H

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace test {
using Test = std::function<void()>;
inline std::vector<std::pair<std::string, Test>> &cases() {
  static std::vector<std::pair<std::string, Test>> values;
  return values;
}
struct Register {
  Register(std::string name, Test body) { cases().emplace_back(std::move(name), std::move(body)); }
};
inline void require(bool condition, const char *expression, const char *file, int line) {
  if (!condition) {
    throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + expression);
  }
}
} // namespace
  // test

#define TEST(name)                                                                                 \
  void name();                                                                                     \
  const test::Register name##_registered(#name, name); /* NOLINT(cert-err58-cpp) */                \
  void name()
#define REQUIRE(expression) test::require((expression), #expression, __FILE__, __LINE__)

#endif // FIND_TEST_SUPPORT_H
