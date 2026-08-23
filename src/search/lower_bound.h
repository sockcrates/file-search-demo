#pragma once

#include "file_io/reader.h"

#include <optional>
#include <string>
#include <string_view>

namespace find::search {

/**
 * @brief Find the first sorted line that is equal to or greater than a term.
 *
 * The reader is interpreted as newline-delimited, ASCII-sorted lines. An
 * unterminated final line is considered a line; newline characters are not part
 * of returned values.
 *
 * @param reader Random-access source containing sorted lines.
 * @param term Search term to use as the lower bound.
 * @return The first line not less than @p term, or `std::nullopt` when none exists.
 * @complexity Logarithmic in file size plus the chunks needed to locate and
 *             compare the visited lines; fixed-buffer memory apart from the
 *             returned line.
 */
[[nodiscard]] std::optional<std::string> lower_bound(const file_io::Reader &reader,
                                                     std::string_view term);

} // namespace
  // find::search
