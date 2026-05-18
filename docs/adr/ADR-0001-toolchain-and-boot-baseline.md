
# ADR-0001 — Toolchain dan Boot Baseline MCSOS 260502

## Status
Accepted for M0 baseline

## Context
MCSOS dikembangkan di Windows 11 x64, tetapi targetnya bare-metal x86_64.
Build system harus portable dan tidak bergantung pada host ABI.

## Decision
1. Build environment utama menggunakan WSL 2 Linux.
2. Repository utama ditempatkan di filesystem Linux WSL (bukan `/mnt/c`).
3. Toolchain M0: LLVM/Clang, LLD, binutils, NASM, Make, CMake, Ninja, Python3.
4. Smoke test menggunakan `clang --target=x86_64-unknown-none`.
5. Emulator utama adalah QEMU system x86_64.
6. Firmware emulator menggunakan OVMF (UEFI).
7. Bootloader awal direkomendasikan Limine (dapat berubah di M1/M2).
8. GCC cross-compiler x86_64-elf bersifat opsional.

## Consequences

### Positive
- Environment lebih konsisten
- Toolchain modern dan mudah diinstall via package manager
- Workflow OS development standar industri

### Trade-offs
- WSL2 menambah layer virtualisasi
- Performa bisa sedikit berbeda antar mesin
- Versi toolchain harus dicatat untuk reproducibility

## Review Trigger
ADR ini harus ditinjau ulang jika:
- arsitektur berubah
- bootloader diganti
- CI diperkenalkan
- toolchain utama berubah
