# Laporan Praktikum M0 — Baseline Requirements, Governance, dan Lingkungan Pengembangan

## 1. Sampul
- Judul praktikum: Praktikum M0 — Baseline Requirements, Governance, dan Lingkungan Pengembangan Reproducible MCSOS 260502
- Nama mahasiswa / kelompok: Syifa Nurzimah
- NIM: 25832074009
- Kelas: 1A
- Dosen: Muhaemin Sidiq, S.Pd., M.Pd.
- Program Studi: Pendidikan Teknologi Informasi, Institut Pendidikan Indonesia
- Tanggal:

## 2. Tujuan
Tujuan praktikum M0 adalah menyiapkan lingkungan pengembangan sistem operasi menggunakan Windows 10 dan WSL 2. Praktikum ini bertujuan memastikan seluruh toolchain seperti Git, Clang, NASM, QEMU, dan GDB dapat digunakan dengan baik sebelum masuk ke tahap pengembangan kernel.

Selain itu, praktikum ini bertujuan memahami konsep baseline requirements, reproducible environment, cross-compilation, dan evidence-first engineering.

## 3. Dasar teori ringkas

### Host dan Target
Host merupakan sistem yang digunakan untuk pengembangan, sedangkan target adalah sistem tujuan hasil kompilasi. Pada praktikum ini host menggunakan Windows 10 dengan WSL2 Ubuntu dan target menggunakan arsitektur x86_64.

### WSL 2
Windows Subsystem for Linux 2 memungkinkan pengguna menjalankan Linux di dalam Windows menggunakan kernel Linux asli.

### Cross Compilation
Cross compilation adalah proses kompilasi program untuk target berbeda dengan sistem host.

### ELF Object
ELF (Executable and Linkable Format) merupakan format file standar pada sistem Linux dan UNIX.

### QEMU
QEMU adalah emulator yang digunakan untuk menjalankan sistem operasi virtual.

### OVMF
OVMF merupakan implementasi firmware UEFI untuk emulator QEMU.

### Git
Git digunakan untuk version control dan melacak perubahan repository.

### Reproducibility
Reproducibility memastikan environment pengembangan dapat direplikasi dengan hasil yang konsisten.

### Evidence-First Engineering
Evidence-first engineering menekankan pentingnya bukti teknis seperti log, metadata, dan output command.

## 4. Lingkungan

| Komponen | Versi / output |
|---|---|
| Windows | Windows 10 |
| WSL distro | Ubuntu |
| Kernel Linux WSL | WSL 2 |
| Git | git version 2.x |
| Clang | clang version 20.x |
| LLD | LLD 20.x |
| binutils/readelf | GNU readelf |
| NASM | NASM |
| QEMU | QEMU emulator version 10.2.1 |
| GDB | GNU gdb 17.1 |
| Python | Python 3 |

Lampirkan isi `build/meta/toolchain-versions.txt`.

## 5. Desain baseline
Repository MCSOS disusun menggunakan struktur direktori terpisah agar dokumentasi, tools, smoke test, dan hasil build mudah dikelola.

Struktur utama repository:
- docs/
- tools/
- smoke/
- build/

Dokumen baseline yang dibuat:
- assumptions and non-goals
- ADR baseline
- threat model
- risk register
- verification matrix
- laporan praktikum

## 6. Langkah kerja
1. Mengaktifkan virtualisasi BIOS.
2. Menginstal WSL2 dan Ubuntu.
3. Menginstal toolchain seperti Git, Clang, NASM, dan QEMU.
4. Membuat repository MCSOS.
5. Membuat struktur direktori baseline.
6. Membuat README dan .gitignore.
7. Membuat script validasi environment.
8. Menjalankan smoke test.
9. Melakukan inspeksi ELF object.
10. Membuat dokumentasi baseline dan laporan.

## 7. Hasil uji

| Pengujian | Command | Hasil | Pass/Fail |
|---|---|---|---|
| WSL version | `wsl --list --verbose` | WSL2 aktif | PASS |
| Tool check | `bash tools/check_env.sh` | Semua tool ditemukan | PASS |
| Metadata | `cat build/meta/toolchain-versions.txt` | Metadata berhasil dibuat | PASS |
| Smoke object | `make smoke` | ELF64 berhasil dibuat | PASS |
| ELF header | `readelf -h build/smoke/freestanding.o` | ELF64 relocatable x86-64 | PASS |
| Git status | `git status` | Repository aktif | PASS |

## 8. Analisis
Selama praktikum terdapat beberapa error terutama pada Makefile. Error “missing separator” terjadi karena Makefile membutuhkan TAB, bukan spasi biasa.

Selain itu terminal sempat masuk ke mode input `cat > Makefile` sehingga command berikutnya dianggap isi file.

Error berhasil diperbaiki dengan membuat ulang Makefile menggunakan indentasi TAB yang benar.

## 9. Keamanan dan reliability
Risiko utama pada tahap M0 meliputi:
- Toolchain mismatch
- Repository path pada `/mnt/c`
- Perbedaan line ending Windows dan Linux
- Kerusakan metadata build

Mitigasi yang dilakukan:
- Repository disimpan pada filesystem Linux WSL
- Menggunakan Git configuration `core.autocrlf input`
- Menggunakan script validasi environment

## 10. Failure modes dan rollback

| Failure mode | Gejala | Diagnosis | Rollback/perbaikan |
|---|---|---|---|
| WSL bukan versi 2 | Toolchain tidak stabil | WSL masih versi lama | Upgrade ke WSL2 |
| Tool tidak ditemukan | command not found | Package belum terinstall | Install package menggunakan apt |
| Repository di `/mnt/c` | Permission bermasalah | Repository berada di filesystem Windows | Pindahkan ke `~/src/mcsos` |
| Smoke object salah target | ELF bukan x86-64 | Target compiler salah | Gunakan `--target=x86_64-unknown-none` |
| OVMF tidak ditemukan | Emulator gagal UEFI | Package firmware belum tersedia | Install package OVMF |

## 11. Kesimpulan
Praktikum M0 berhasil menyiapkan baseline environment pengembangan sistem operasi menggunakan Windows 10, WSL2 Ubuntu, dan toolchain x86_64.

Tahap M0 hanya membuktikan environment siap diuji dan belum membuktikan kernel dapat melakukan booting.

## 12. Lampiran
- Output `tools/check_env.sh`
- Isi `build/meta/toolchain-versions.txt`
- Output `readelf -h`
- Output `objdump`
- Screenshot relevan
- Commit hash

## 13. Referensi

[1] Microsoft, “Windows Subsystem for Linux Documentation,” Microsoft Learn, 2025.

[2] LLVM Project, “Clang Compiler User Manual,” LLVM Documentation, 2025.

[3] QEMU Project, “QEMU Documentation,” 2025.

[4] Free Software Foundation, “GNU Make Manual,” GNU Project, 2025.

[5] Git SCM, “Git Documentation,” 2025.
