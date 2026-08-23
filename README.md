# find

`find <search-term> <filename>` prints the first sorted ASCII line equal to or greater than the term. It exits `0` when found, `1` when no such line exists, and `2` for usage or I/O errors.

Build and test with `mise run check`; use `mise run sanitizer` for AddressSanitizer and UndefinedBehaviorSanitizer.

The executable is a modular monolith. `cli` owns argument errors, `file_io` exposes the single random-access reader abstraction, `line_scanning` finds boundaries and compares incrementally, and `search` implements lower-bound binary search. The search invariant is that every line before `low` is smaller than the term and `best` is the earliest known candidate.

Reads are chunked (64 KiB) and positioned, so no whole file or whole candidate line is loaded during searching. A final line without a newline is a valid line; newlines terminate the preceding line and consecutive newlines represent empty lines. Runtime is logarithmic in file size plus the chunks needed to resolve and compare visited lines; memory use is bounded by fixed read buffers, aside from the returned output line.
