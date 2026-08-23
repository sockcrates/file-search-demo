#pragma once

#include "file_io/reader.h"

#include <optional>
#include <string>
#include <string_view>

namespace find::search {

[[nodiscard]] std::optional<std::string> lower_bound(const file_io::Reader &reader,
                                                     std::string_view term);

} // namespace
  // find::search
