#!/usr/bin/env bash
set -euo pipefail

installer_path="${RUNNER_TEMP:?RUNNER_TEMP must be set}/llvm.sh"
curl --fail --location --retry 3 --retry-all-errors --silent \
  https://apt.llvm.org/llvm.sh --output "$installer_path"
chmod +x "$installer_path"
sudo "$installer_path" 21
sudo apt-get install --yes clang-format-21 clang-tidy-21

llvm_bin="/usr/lib/llvm-21/bin"
test -x "$llvm_bin/clang++"
test -x "$llvm_bin/clang-format"
test -x "$llvm_bin/clang-tidy"
printf '%s\n' "$llvm_bin" >> "${GITHUB_PATH:?GITHUB_PATH must be set}"
