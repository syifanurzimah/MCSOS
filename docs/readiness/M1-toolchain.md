# Readiness Review M1 - Toolchain Reproducible

## Identitas

* Nama mahasiswa/kelompok: Syifa Nurzimah
* NIM anggota: 25832074009
* Kelas: 1A
* Dosen: Muhaemin Sidiq, S.Pd., M.Pd.
* Program Studi: Pendidikan Teknologi Informasi, Institut Pendidikan Indonesia
* Tanggal: 31 Mei 2026
* Commit hash: f96a91719cfb5f404403ce1e4ff44d47a601429e

## Ringkasan hasil

Praktikum M1 berhasil diselesaikan. Lingkungan pengembangan menggunakan WSL 2 Ubuntu telah dikonfigurasi dengan baik, seluruh toolchain yang dipersyaratkan tersedia, proof build freestanding berhasil dibuat, QEMU dan OVMF berhasil terdeteksi, serta pengujian reproducibility menghasilkan hash yang identik pada dua kali build. Berdasarkan hasil pengujian, milestone M1 dinyatakan siap untuk melanjutkan ke M2.

## Evidence checklist

| Evidence                | Path                                                         | Status | Catatan                    |
| ----------------------- | ------------------------------------------------------------ | ------ | -------------------------- |
| Toolchain versions      | `build/meta/toolchain-versions.txt`                          | OK     | File berhasil dibuat       |
| Host readiness          | `build/meta/host-readiness.txt`                              | OK     | Host memenuhi syarat       |
| QEMU capabilities       | `build/meta/qemu-capabilities.txt`                           | OK     | QEMU dan OVMF terdeteksi   |
| Freestanding object     | `build/proof/freestanding_probe.o`                           | OK     | Berhasil dibuat            |
| Freestanding ELF        | `build/proof/freestanding_probe.elf`                         | OK     | Berhasil dibuat            |
| ELF header              | `build/proof/readelf-header.txt`                             | OK     | ELF64 x86_64               |
| ELF sections            | `build/proof/readelf-sections.txt`                           | OK     | Berhasil diperiksa         |
| Disassembly             | `build/proof/objdump-disassembly.txt`                        | OK     | Berhasil dibuat            |
| Undefined symbol report | `build/proof/nm-undefined.txt`                               | OK     | Tidak ada undefined symbol |
| Reproducibility hash    | `build/repro/sha256-run1.txt`, `build/repro/sha256-run2.txt` | OK     | Hash identik               |

## Acceptance criteria M1

| Kriteria                                  | Lulus/Gagal | Bukti                              |
| ----------------------------------------- | ----------- | ---------------------------------- |
| Repository berada di filesystem Linux WSL | Lulus       | `/home/syifa/src/mcsos`            |
| Semua tool wajib tersedia                 | Lulus       | Output `make check`                |
| `make meta` berhasil                      | Lulus       | Metadata berhasil dibuat           |
| `make check` berhasil                     | Lulus       | Semua tool terdeteksi              |
| `make proof` berhasil                     | Lulus       | Object dan ELF berhasil dibuat     |
| `make qemu-probe` berhasil                | Lulus       | QEMU dan OVMF terdeteksi           |
| `make repro` berhasil                     | Lulus       | Hash build identik                 |
| `make test` berhasil dari clean checkout  | Lulus       | Output "OK: M1 test suite passed"  |
| `nm-undefined.txt` kosong                 | Lulus       | Tidak ada simbol tidak terdefinisi |
| Hasil `readelf` menunjukkan ELF64 x86_64  | Lulus       | Header ELF valid                   |

## Known limitations

1. Belum menggunakan cross compiler `x86_64-elf-gcc`.
2. Belum tersedia Continuous Integration (CI).
3. Belum ada pengujian pada perangkat keras nyata.
4. Belum membuat bootable image atau kernel yang dapat dijalankan.
5. Pengujian masih terbatas pada proof build toolchain.

## Risiko dan mitigasi

| Risiko                                                    | Mitigasi                                       |
| --------------------------------------------------------- | ---------------------------------------------- |
| Compiler menggunakan target yang salah                    | Verifikasi dengan `readelf` dan target triple  |
| Repository berada di `/mnt/c`                             | Menyimpan repository pada filesystem Linux WSL |
| Perbedaan versi toolchain menyebabkan hasil build berubah | Mencatat seluruh versi toolchain pada metadata |
| Artifact hasil build ikut ter-commit                      | Menggunakan `.gitignore` dan `make distclean`  |
| QEMU atau OVMF tidak tersedia                             | Memverifikasi dengan `make qemu-probe`         |

## Readiness decision

* [ ] Belum siap lanjut M2.
* [ ] Siap lanjut M2 dengan catatan.
* [x] Siap lanjut M2.

Alasan keputusan:

Seluruh acceptance criteria M1 telah terpenuhi. Toolchain, proof build, reproducibility check, serta validasi QEMU/OVMF berhasil dijalankan tanpa error. Evidence yang diperlukan telah tersedia sehingga milestone M1 dinyatakan siap untuk melanjutkan ke M2.

