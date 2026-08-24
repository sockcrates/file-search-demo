#!/usr/bin/env bash
set -euo pipefail

readonly key_source="scripts/ci/keys/llvm-snapshot.gpg"
readonly key_fingerprint="6084F3CF814B57C1CF12EFD515CF4D18AF4F7421"
readonly keyring_path="/etc/apt/keyrings/llvm-archive-keyring.gpg"
readonly source_path="/etc/apt/sources.list.d/llvm.list"
readonly source_entry="deb [arch=amd64 signed-by=$keyring_path] https://apt.llvm.org/noble/ llvm-toolchain-noble-21 main"

test -r "$key_source"
actual_fingerprint="$(gpg --show-keys --with-colons "$key_source" | awk -F: '$1 == "fpr" { print $10; exit }')"
test "$actual_fingerprint" = "$key_fingerprint"
gpg --dearmor < "$key_source" > "${RUNNER_TEMP:?RUNNER_TEMP must be set}/llvm-archive-keyring.gpg"
sudo install --directory --mode 0755 /etc/apt/keyrings
sudo install --mode 0644 "${RUNNER_TEMP}/llvm-archive-keyring.gpg" "$keyring_path"
printf '%s\n' "$source_entry" | sudo tee "$source_path" > /dev/null

sudo apt-get update
# libclang-rt-21-dev provides the AddressSanitizer and UndefinedBehaviorSanitizer
# runtimes used by clang++ when the sanitizer CI job links its executables.
sudo apt-get install --yes --no-install-recommends clang-21 clang-format-21 clang-tidy-21 libclang-rt-21-dev

llvm_bin="/usr/lib/llvm-21/bin"
test -x "$llvm_bin/clang++"
test -x "$llvm_bin/clang-format"
test -x "$llvm_bin/clang-tidy"
asan_runtime="$("$llvm_bin/clang++" -print-file-name=libclang_rt.asan.a)"
test "$asan_runtime" != "libclang_rt.asan.a"
test -f "$asan_runtime"
printf '%s\n' "$llvm_bin" >> "${GITHUB_PATH:?GITHUB_PATH must be set}"
