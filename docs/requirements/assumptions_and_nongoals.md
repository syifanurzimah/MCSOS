# Assumptions and Non-Goals MCSOS 260502 — M0

## Assumptions
1. Target arsitektur awal adalah x86_64 long mode.
2. Host pengembangan adalah Windows 11 x64.
3. Build dilakukan di WSL 2 Linux environment.
4. Repository utama berada di filesystem Linux WSL.
5. Emulator utama adalah QEMU system x86_64.
6. Firmware emulator menggunakan OVMF/UEFI.
7. Bootloader awal menggunakan Limine atau setara.
8. Kernel awal menggunakan freestanding C17 + assembly minimal.
9. Target kompatibilitas adalah POSIX-like subset (bukan Linux ABI).
10. Semua milestone harus memiliki evidence (log, screenshot, dump, dll).

## Non-Goals M0
1. M0 tidak membuat kernel bootable.
2. M0 tidak mengimplementasikan bootloader.
3. M0 tidak membuat linker script final.
4. M0 tidak mengimplementasikan interrupt, paging, scheduler, syscall, VFS, driver, networking, graphics.
5. M0 tidak menargetkan production-ready kernel.
6. M0 tidak menjamin kompatibilitas semua hardware.
7. M0 tidak membutuhkan bare-metal hardware.
8. M0 tidak menuntut build 100% deterministic, hanya terdokumentasi.
