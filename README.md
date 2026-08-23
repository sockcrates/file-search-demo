# find

`find` performs a lower-bound search over a sorted, newline-delimited ASCII file.
It prints the first line equal to or greater than a search term without copying
the whole file into application-managed memory.

## Usage

```console
$ find <search-term> <filename>
```

For example, with a vegetable file containing broccoli, carrot, and celery:

```console
$ find carr vegetables.txt
apricot
```

Exit codes are:

| Code | Meaning |
| --- | --- |
| `0` | A matching lower-bound line was found. |
| `1` | No line is equal to or greater than the search term. |
| `2` | The invocation is invalid or the file could not be read. |

## Requirements and development

The project uses C++23, CMake, and a Clang toolchain. [mise](https://mise.jdx.dev/)
can provision the pinned developer tools.

```console
$ mise run check       # Format check, strict compilation, and tests
$ mise run sanitizer   # AddressSanitizer and UndefinedBehaviorSanitizer
$ mise run benchmark   # Search workloads and I/O metrics
```

To build directly with CMake:

```console
$ cmake -S . -B build -DCMAKE_CXX_COMPILER=clang++
$ cmake --build build
$ ./build/find carr vegetables.txt
```

## Behaviour and performance

Input must be sorted in unsigned ASCII byte order. A final line without a
trailing newline is valid. Consecutive newlines represent empty lines, and a
newline terminates the preceding line rather than belonging to it.

The search performs binary search by file offset. Regular files use a read-only
memory mapping and compare only the visited lines; generic readers use bounded
positioned reads. Its runtime is logarithmic in file size plus the chunks needed
to resolve those lines. Mapped input must remain stable while the search runs;
concurrent truncation is unsupported. Generic-reader memory use is bounded by
fixed read buffers, apart from the returned output line.

## Architecture

- `cli` parses command-line arguments and owns usage errors.
- `file_io` provides the `Reader` random-access abstraction and file/memory implementations.
- `line_scanning` locates line ranges and compares or materializes them incrementally.
- `search` implements the lower-bound binary search.

The key search invariant is that every line before `low` is less than the term,
while `best`, when present, is the earliest known candidate.

## API documentation

Public C++ APIs use Doxygen documentation. The project follows the documented
comment and formatting convention in [docs/doxygen.md](docs/doxygen.md).

When Doxygen is installed, generate the HTML API reference with:

```console
$ cmake --build build --target docs
```

The generated site is written to `build/docs/html/index.html`.
