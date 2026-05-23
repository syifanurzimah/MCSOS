#!/usr/bin/env bash

set -e

OUTDIR="build/evidence/M0"

mkdir -p "$OUTDIR"

echo "[*] Collecting evidence..."

bash tools/check_env.sh > "$OUTDIR/check_env.txt"

make smoke > "$OUTDIR/make_smoke.txt"

git log --oneline -n 3 > "$OUTDIR/git_log.txt"

git status --short > "$OUTDIR/git_status.txt"

readelf -h build/smoke/freestanding.o > "$OUTDIR/readelf.txt"

file build/smoke/freestanding.o > "$OUTDIR/file.txt"

echo "[OK] Evidence collected in $OUTDIR"
