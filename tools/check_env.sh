#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
META_DIR="$ROOT_DIR/build/meta"

mkdir -p "$META_DIR"

fail=0

say() {
  printf '[M0] %s\n' "$*"
}

check_tool() {
  local tool="$1"
  if command -v "$tool" >/dev/null 2>&1; then
    printf '[OK] %-24s %s\n' "$tool" "$(command -v "$tool")"
  else
    printf '[FAIL] %-24s not found\n' "$tool"
    fail=1
  fi
}

say "Repository root: $ROOT_DIR"

case "$ROOT_DIR" in
  /mnt/c/*|/mnt/d/*|/mnt/e/*)
    printf '[WARN] Repository is on Windows filesystem\n'
    ;;
  *)
    printf '[OK] Repository is in Linux filesystem\n'
    ;;
esac

say "Checking tools"

for tool in git make clang ld.lld llvm-readelf llvm-objdump readelf objdump nasm qemu-system-x86_64 gdb python3 shellcheck cppcheck; do
  check_tool "$tool"
done

if [ "$fail" -ne 0 ]; then
  exit 1
fi

say "Done"
