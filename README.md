# MCSOS 260502

MCSOS adalah proyek sistem operasi pendidikan bertahap untuk target x86_64 menggunakan WSL2 sebagai environment pengembangan.

## Status
M0 baseline: environment development sudah siap.

## Target
- Arsitektur: x86_64
- Emulator: QEMU system-x86_64
- Firmware: OVMF (UEFI)
- Bahasa: C17 freestanding + assembly x86_64

## Struktur
- docs/
- tools/
- build/

## Validasi awal
- make meta
- make check
- make smoke

## Catatan
M0 hanya membuktikan environment siap, belum kernel boot.
