 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M0

**Nama file laporan:** `laporan_praktikum_m13_25832074009.md`  
**Nama sistem operasi:** MCSOS versi 260502  
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset  
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.  
**Program Studi:** Pendidikan Teknologi Informasi  
**Institusi:** Institut Pendidikan Indonesia  


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M13]` |
| Judul praktikum | `[Virtual File System (VFS), RAMFS, dan File Descriptor Layer]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[-]` |
| Anggota kelompok | `[-]` |
| Tanggal praktikum | `[2026-07-07]` |
| Tanggal pengumpulan | `[2026-07-08]` |
| Repository | `[https://github.com/syifanurzimah/MCSOS]` |
| Branch | `[praktikum-m13-vfs-ramfs]` |
| Commit awal | `` `[673ea92]` `` |
| Commit akhir | `` `[57d5783]` `` |
| Status readiness yang diklaim | `[Kandidat siap pakai terbatas]` |

---

## 1. Sampul

# Laporan Praktikum `M13`  
## `Virtual File System (VFS), RAMFS, dan File Descriptor Layer`

Disusun oleh:

| Nama | NIM | Kelas | Peran |
|---|---|---|---|
| `[Syifa Nurzimah]` | `[25832074009]` | `[1A]` | `[individu]` |
| `[opsional]` | `[opsional]` | `[opsional]` | `[opsional]` |

Dosen Pengampu: **Muhaemin Sidiq, S.Pd., M.Pd.**  
Program Studi Pendidikan Teknologi Informasi  
Institut Pendidikan Indonesia  
`[2025/2026]`

---

## 2. Pernyataan Orisinalitas dan Integritas Akademik

| Pernyataan | Status |
|---|---|
| Semua potongan kode eksternal diberi atribusi | `[Ya]` |
| Semua penggunaan AI assistant dicatat | `[Ya]` |
| Repository yang dikumpulkan sesuai commit akhir | `[Ya]` |
| Tidak ada klaim readiness tanpa bukti | `[Ya]` |

Catatan penggunaan bantuan eksternal:

```text
Praktikum dikerjakan secara mandiri dengan bantuan dokumentasi resmi
Clang, GNU Make, GNU Binutils (nm, readelf, objdump), serta AI Assistant
(Claude) untuk membantu menjelaskan konsep VFS/RAMFS/file descriptor,
memperbaiki error build, serta membantu penyusunan laporan.

Seluruh kode yang dihasilkan diverifikasi kembali dengan proses build
host test, kompilasi objek freestanding x86_64-elf, audit undefined
symbol (nm -u), serta inspeksi ELF (readelf, objdump) hingga host test
menghasilkan status PASS dan tidak ditemukan undefined symbol.
```

---

## 3. Tujuan Praktikum

Tuliskan tujuan teknis dan konseptual praktikum. Tujuan harus dapat diuji.

1. `Membangun modul Virtual File System (VFS) MCSOS milestone M13 yang menyediakan abstraksi file dan operasi dasar filesystem.`
2. `Mengimplementasikan RAMFS sebagai backend filesystem in-memory untuk VFS.`
3. `Mengimplementasikan lapisan file descriptor (FD) untuk manajemen deskriptor file pada level kernel.`
4. `Menyediakan antarmuka syscall VFS (sys_vfs) sebagai jembatan antara pemanggil dan lapisan VFS/RAMFS/FD.`
5. `Memvalidasi implementasi menggunakan host unit test serta audit objek ELF freestanding (nm, readelf, objdump) sebelum diintegrasikan ke kernel penuh.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Mampu merancang dan mengimplementasikan lapisan VFS dengan backend RAMFS]` | `[host test PASS, build/m13/vfs.o]` |
| `[Mampu mengelola file descriptor pada level kernel]` | `[kernel/vfs/fd.c, kernel.syms/nm]` |
| `[Mampu melakukan audit ELF objek freestanding sebelum integrasi kernel]` | `[nm -u kosong, readelf-vfs.txt, objdump-vfs.txt]` |

---

## 5. Peta Milestone MCSOS

Centang milestone yang menjadi fokus laporan ini. Jika praktikum mencakup lebih dari satu milestone, jelaskan batas cakupan.

| Milestone | Fokus | Status dalam laporan |
|---|---|---|
| M0 | Requirements, governance, baseline arsitektur | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M1 | Toolchain reproducible, Git, QEMU, GDB, metadata build | `[ ] tidak dibahas / [ x ] dibahas / [  ] selesai praktikum` |
| M2 | Boot image, kernel ELF64, early console | `[ ] tidak dibahas / [ x ] dibahas / [  ] selesai praktikum` |
| M3 | Panic path, linker map, GDB, observability awal | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M4 | Trap, exception, interrupt, timer | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M5 | PMM, VMM, page table, kernel heap | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M6 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M7 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M8 | VFS, file descriptor, ramfs | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M9 | Block layer dan device model | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M10 | Persistent filesystem, mcsfs/ext2-like, recovery | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M11 | Networking stack, packet parsing, UDP/TCP subset | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M12 | Security model, capability/ACL, syscall fuzzing, hardening | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M13 | SMP, scalability, lock stress, NUMA-aware preparation | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M14 | Framebuffer, graphics console, visual regression | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M15 | Virtualization/container subset | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Batas cakupan praktikum:

```text
Praktikum M13 pada repository ini berfokus pada implementasi VFS
(Virtual File System), backend RAMFS in-memory, lapisan file
descriptor (FD), serta antarmuka syscall VFS (sys_vfs), yang secara
konten sejalan dengan deskripsi milestone "M8 - VFS, file descriptor,
ramfs" pada peta milestone acuan di atas. Penomoran M13 pada branch
Git (praktikum-m13-vfs-ramfs) mengikuti urutan praktikum lokal
mahasiswa/kelas, bukan urutan pada peta milestone acuan generik,
sehingga terdapat perbedaan nomor antara peta milestone acuan dan
penomoran praktikum lokal. Hal ini dicatat secara eksplisit agar
tidak menimbulkan kekeliruan interpretasi.

Praktikum ini mencakup pembuatan header VFS, implementasi RAMFS, FD,
sys_vfs, unit test host, kompilasi objek freestanding x86_64-elf, dan
audit ELF objek (nm, readelf, objdump). Praktikum ini BELUM mencakup
integrasi ke kmain()/kernel.elf penuh, pembuatan image ISO, QEMU
smoke test, maupun debugging menggunakan GDB, sehingga bukti pada
bagian-bagian tersebut ditandai NA (belum dilakukan) pada laporan
ini.
```

---

## 6. Dasar Teori Ringkas

Tuliskan teori yang langsung diperlukan untuk memahami praktikum. Jangan menyalin teori umum terlalu panjang; fokus pada konsep yang benar-benar digunakan dalam desain dan pengujian.

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Pada praktikum M13, konsep utama yang dipelajari adalah abstraksi
Virtual File System (VFS) sebagai lapisan generik yang memisahkan
antarmuka file dari implementasi backend filesystem tertentu, RAMFS
sebagai backend filesystem sederhana yang menyimpan data di memori,
serta lapisan file descriptor (FD) yang memetakan deskriptor angka ke
objek file/handle pada kernel. Selain itu dipelajari pola penyediaan
antarmuka syscall (sys_vfs) sebagai titik masuk operasi filesystem,
dan proses audit objek ELF relocatable sebelum digabungkan ke kernel
utuh.
```

### 6.2 Konsep Arsitektur x86_64 yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[ELF64 Relocatable (REL)]` | `[Objek ramfs.o, fd.o, sys_vfs.o, dan hasil ld -r vfs.o berbentuk ELF64 REL]` | `[readelf -h]` |
| `[Target freestanding x86_64-elf]` | `[Kompilasi kernel/vfs/*.c menggunakan clang -target x86_64-elf -ffreestanding]` | `[log build Makefile.m13]` |
| `[System V x86_64 ABI]` | `[Konvensi pemanggilan fungsi VFS/FD/sys_vfs]` | `[objdump-vfs.txt]` |
| `[Symbol resolution / undefined symbol]` | `[Memastikan tidak ada undefined symbol pada vfs.o hasil link]` | `[nm -u, build/m13/nm-undefined.txt kosong]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding]` |
| Runtime | `[tanpa hosted libc pada target kernel; host test memakai libc host untuk mempercepat iterasi]` |
| ABI | `[x86_64 System V ABI]` |
| Risiko undefined behavior | `[Pointer invalid pada node RAMFS, off-by-one pada tabel file descriptor, integer overflow pada ukuran file, dan kesalahan penanganan symbol saat linking objek relocatable]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[POSIX.1 file interface concepts]` | `[Konsep file descriptor dan operasi file dasar]` | `[Acuan desain antarmuka VFS/FD]` |
| `[2]` | `[ELF Specification]` | `[ELF Header, Section Header, Symbol Table]` | `[Analisis objek relocatable vfs.o]` |
| `[3]` | `[Clang/LLVM Documentation]` | `[Target freestanding, flag -ffreestanding]` | `[Kompilasi kernel/vfs/*.c untuk x86_64-elf]` |
| `[4]` | `[GNU Binutils Documentation]` | `[nm, readelf, objdump, ld -r]` | `[Audit dan penggabungan objek freestanding]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows 10/11 x64]` |
| Lingkungan build | `[WSL 2 Ubuntu]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-unknown-none-elf (target clang: x86_64-elf)]` |
| Emulator | `[QEMU (tidak digunakan pada sesi M13 ini)]` |
| Firmware emulator | `[OVMF (tidak digunakan pada sesi M13 ini)]` |
| Debugger | `[GNU GDB (tidak digunakan pada sesi M13 ini)]` |
| Build system | `[GNU Make (Makefile.m13)]` |
| Bahasa utama | `[C17 Freestanding, host test memakai C17 hosted untuk kompilasi cc]` |
| Assembly | `[Tidak digunakan pada M13]` |

### 7.2 Versi Toolchain

```text
Versi toolchain tidak diverifikasi ulang secara eksplisit pada sesi
terminal M13 ini (tidak dijalankan `clang --version`, `gcc --version`,
dsb. pada sesi ini). Berdasarkan riwayat perintah aktual yang
dijalankan, toolchain yang benar-benar digunakan pada sesi ini adalah:

- cc (untuk kompilasi host test)
- clang dengan target x86_64-elf (untuk objek freestanding)
- ld (GNU ld, untuk `ld -r -m elf_x86_64`)
- nm, readelf, objdump (GNU Binutils, untuk audit ELF)
- sha256sum (untuk checksum artefak)
- git (untuk version control)

Verifikasi versi lengkap toolchain terakhir kali tercatat pada
praktikum M1 dan diasumsikan masih berlaku pada environment WSL 2
yang sama.
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[~/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[praktikum-m13-vfs-ramfs]` |
| Commit hash awal | `` `[673ea92]` `` |
| Commit hash akhir | `` `[57d5783]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

Tampilkan hanya direktori dan file yang relevan dengan praktikum.

```text
mcsos/
├── kernel/
│   └── vfs/
│       ├── ramfs.c
│       ├── fd.c
│       └── sys_vfs.c
├── include/
│   └── mcs_vfs.h
├── tests/
│   └── m13_vfs_host_test.c
├── build/
│   └── m13/
├── Makefile.m13
└── Makefile
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[Makefile.m13]` | `[baru]` | `[Target build khusus M13: host test dan kompilasi objek freestanding]` | `[rendah]` |
| `[include/mcs_vfs.h]` | `[baru]` | `[Header definisi struct dan API VFS]` | `[sedang]` |
| `[kernel/vfs/ramfs.c]` | `[baru]` | `[Implementasi backend RAMFS in-memory]` | `[sedang]` |
| `[kernel/vfs/fd.c]` | `[baru]` | `[Implementasi lapisan file descriptor]` | `[sedang]` |
| `[kernel/vfs/sys_vfs.c]` | `[baru]` | `[Antarmuka syscall VFS]` | `[sedang]` |
| `[tests/m13_vfs_host_test.c]` | `[baru]` | `[Unit test host untuk VFS/FD/RAMFS]` | `[rendah]` |

### 8.3 Ringkasan Diff

```bash
git status --short
git diff --stat
git log --oneline -n 5
```

Output relevan (sebelum commit):

```text
?? Makefile.m13
?? include/mcs_vfs.h
?? kernel/vfs/
?? tests/m13_vfs_host_test.c
```

Output relevan (setelah commit dan push):

```text
[praktikum-m13-vfs-ramfs 57d5783] M13: implement VFS RAMFS and file descriptor layer
 6 files changed, 759 insertions(+)
 create mode 100644 Makefile.m13
 create mode 100644 include/mcs_vfs.h
 create mode 100644 kernel/vfs/fd.c
 create mode 100644 kernel/vfs/ramfs.c
 create mode 100644 kernel/vfs/sys_vfs.c
 create mode 100644 tests/m13_vfs_host_test.c

On branch praktikum-m13-vfs-ramfs
Your branch is up to date with 'origin/praktikum-m13-vfs-ramfs'.
nothing to commit, working tree clean
```

---

## 9. Desain Teknis

### 9.1 Masalah yang Diselesaikan

```text
Kernel pada milestone sebelumnya (M12: synchronization primitives dan
lockdep) belum memiliki lapisan filesystem untuk menyimpan dan
mengelola file. Pada praktikum M13 ditambahkan lapisan VFS generik,
backend RAMFS in-memory, lapisan file descriptor, serta antarmuka
syscall VFS, sehingga kernel memiliki fondasi awal untuk operasi
file.
```

### 9.2 Keputusan Desain

| Keputusan | Alternatif yang dipertimbangkan | Alasan memilih | Konsekuensi |
|---|---|---|---|
| `[RAMFS sebagai backend awal VFS]` | `[Filesystem persisten (mcsfs/ext2-like)]` | `[Lebih sederhana untuk membangun fondasi VFS terlebih dahulu]` | `[Data tidak persisten, hilang saat reboot]` |
| `[Lapisan VFS generik terpisah dari backend]` | `[Akses langsung ke RAMFS tanpa abstraksi]` | `[Memudahkan penambahan backend filesystem lain di masa depan]` | `[Ada overhead lapisan abstraksi]` |
| `[File descriptor table terpisah dari VFS core (fd.c)]` | `[Menyatukan FD langsung di VFS core]` | `[Modularitas kode dan mempermudah pengujian terpisah]` | `[Perlu sinkronisasi/kontrak yang jelas antar modul]` |
| `[Validasi tanpa QEMU, cukup host test + audit ELF]` | `[Langsung integrasi ke kmain dan uji QEMU]` | `[Mempercepat iterasi desain sebelum integrasi kernel penuh]` | `[Belum ada bukti perilaku pada kernel nyata/QEMU]` |

### 9.3 Arsitektur Ringkas

Tambahkan diagram ASCII atau Mermaid. Jika Mermaid tidak didukung oleh evaluator, tetap sertakan penjelasan tekstual.

```mermaid
Pemanggil (rencana: user syscall / kmain)
     │
     ▼
sys_vfs.c (antarmuka syscall VFS)
     │
     ▼
fd.c (lapisan file descriptor)
     │
     ▼
ramfs.c (backend filesystem in-memory)
```

Penjelasan diagram:

```text
Antarmuka sys_vfs.c berperan sebagai titik masuk operasi filesystem.
Permintaan operasi file diteruskan ke lapisan file descriptor (fd.c)
untuk pemetaan deskriptor ke objek file, yang kemudian berinteraksi
dengan backend RAMFS (ramfs.c) untuk operasi baca/tulis/metadata di
memori. Pada tahap M13 ini, jalur dari kmain()/syscall kernel yang
sesungguhnya ke sys_vfs.c belum dihubungkan; pengujian dilakukan
melalui host test yang memanggil ketiga modul secara langsung.
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[sys_vfs_*()]` | `[host test / rencana: syscall kernel]` | `[fd.c]` | `[Parameter valid]` | `[Operasi file diteruskan]` | `[Kode error dikembalikan]` |
| `[fd alloc/release]` | `[sys_vfs.c]` | `[fd.c]` | `[Tabel FD terinisialisasi]` | `[Deskriptor valid dialokasikan/dilepas]` | `[Gagal jika tabel penuh]` |
| `[ramfs operasi file]` | `[fd.c]` | `[ramfs.c]` | `[Node RAMFS valid]` | `[Data terbaca/tertulis di memori]` | `[Gagal jika node tidak ditemukan]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `[tabel file descriptor]` `` | `[indeks fd, pointer objek file]` | `[fd.c]` | `[Selama proses/kernel aktif]` | `[Tidak boleh melebihi kapasitas tabel]` |
| `` `[node RAMFS]` `` | `[nama, data, ukuran, parent]` | `[ramfs.c]` | `[Selama filesystem in-memory aktif]` | `[Node selain root selalu memiliki parent valid]` |

### 9.6 Invariants

Tuliskan invariant yang harus benar sepanjang eksekusi.

1. `Objek hasil link vfs.o tidak boleh memiliki undefined symbol (diverifikasi dengan nm -u).`
2. `Setiap file descriptor yang dialokasikan harus mengacu pada node RAMFS yang valid.`
3. `Operasi sys_vfs tidak boleh langsung memanipulasi node RAMFS tanpa melalui lapisan fd.c.`
4. `Host test m13_vfs_host_test harus mengembalikan status PASS sebelum objek freestanding dianggap siap diaudit.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[tabel file descriptor]` | `[fd.c]` | `[belum ada]` | `[belum diverifikasi]` | `[Belum terintegrasi dengan mutex/spinlock M12]` |
| `[node RAMFS]` | `[ramfs.c]` | `[belum ada]` | `[belum diverifikasi]` | `[Belum terintegrasi dengan mutex/spinlock M12]` |

Lock order yang berlaku:

```text
Pada tahap M13 ini, modul VFS/RAMFS/FD belum diintegrasikan dengan
primitif sinkronisasi (mutex/spinlock/lockdep) yang telah dibangun
pada milestone M12. Integrasi locking direncanakan pada iterasi
berikutnya setelah VFS diintegrasikan ke kmain()/kernel penuh.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Undefined symbol saat linking objek]` | `[ld -r vfs.o]` | `[Audit dengan nm -u]` | `[build/m13/nm-undefined.txt kosong]` |
| `[Pointer invalid pada node RAMFS]` | `[ramfs.c]` | `[Validasi node sebelum akses (rencana pengujian lanjutan)]` | `[host test m13_vfs_host_test.c]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[Pemanggil → sys_vfs]` | `[Parameter operasi file]` | `[Direncanakan divalidasi di sys_vfs.c]` | `[Mengembalikan kode error]` |
| `[fd.c → ramfs.c]` | `[Indeks file descriptor]` | `[Pemeriksaan batas tabel FD]` | `[Operasi ditolak jika FD tidak valid]` |

---

## 10. Langkah Kerja Implementasi

Gunakan tabel berikut untuk setiap langkah. Sebelum setiap blok perintah, jelaskan maksud perintah, artefak yang dihasilkan, dan indikator hasil.

### Langkah 1 — `Membuat branch praktikum M13`

Maksud langkah:

```text
Membuat branch kerja baru untuk praktikum M13 dari branch M12 yang
sudah selesai (praktikum/m12-sync).
```

Perintah:

```bash
git checkout -b praktikum-m13-vfs-ramfs
```

Output ringkas:

```text
Switched to a new branch 'praktikum-m13-vfs-ramfs'
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Branch baru]` | `[Git]` | `[Ruang kerja terisolasi untuk M13]` |

Indikator berhasil:

```text
Branch praktikum-m13-vfs-ramfs berhasil dibuat dan aktif.
```

### Langkah 2 — `Membuat struktur direktori kerja`

Maksud langkah:

```text
Menyiapkan direktori yang dibutuhkan untuk header, implementasi VFS,
test, dan hasil build M13.
```

Perintah:

```bash
mkdir -p include kernel/vfs tests build/m13
```

Output ringkas:

```text
(tidak ada output; direktori berhasil dibuat)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Direktori kernel/vfs]` | `[kernel/vfs]` | `[Tempat implementasi VFS/RAMFS/FD]` |
| `[Direktori build/m13]` | `[build/m13]` | `[Tempat artefak build M13]` |

Indikator berhasil:

```text
Struktur direktori include, kernel/vfs, tests, dan build/m13 tersedia.
```

### Langkah 3 — `Menulis header VFS`

Maksud langkah:

```text
Mendefinisikan struct dan API VFS pada header bersama.
```

Perintah:

```bash
nano include/mcs_vfs.h
```

Output ringkas:

```text
git status menunjukkan include/mcs_vfs.h sebagai untracked file baru.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[mcs_vfs.h]` | `[include/]` | `[Header definisi VFS]` |

Indikator berhasil:

```text
File include/mcs_vfs.h berhasil dibuat.
```

### Langkah 4 — `Menulis implementasi RAMFS`

Maksud langkah:

```text
Mengimplementasikan backend filesystem in-memory (RAMFS).
```

Perintah:

```bash
nano kernel/vfs/ramfs.c
```

Output ringkas:

```text
git status menunjukkan direktori kernel/vfs/ sebagai untracked.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[ramfs.c]` | `[kernel/vfs/]` | `[Implementasi backend RAMFS]` |

Indikator berhasil:

```text
File kernel/vfs/ramfs.c berhasil dibuat.
```

### Langkah 5 — `Menulis implementasi file descriptor`

Maksud langkah:

```text
Mengimplementasikan lapisan file descriptor.
```

Perintah:

```bash
nano kernel/vfs/fd.c
```

Output ringkas:

```text
git status tetap menunjukkan kernel/vfs/ sebagai untracked (fd.c ikut di dalamnya).
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[fd.c]` | `[kernel/vfs/]` | `[Implementasi lapisan file descriptor]` |

Indikator berhasil:

```text
File kernel/vfs/fd.c berhasil dibuat.
```

### Langkah 6 — `Menulis antarmuka syscall VFS`

Maksud langkah:

```text
Menyediakan antarmuka syscall VFS (sys_vfs) sebagai titik masuk
operasi filesystem.
```

Perintah:

```bash
nano kernel/vfs/sys_vfs.c
```

Catatan proses:

```text
Percobaan pertama sempat salah ketik dengan menjalankan langsung
`kernel/vfs/sys_vfs.c` sebagai perintah shell (tanpa `nano`), sehingga
muncul error:
-bash: kernel/vfs/sys_vfs.c: No such file or directory
Perintah kemudian diulang dengan benar menggunakan `nano
kernel/vfs/sys_vfs.c`.
```

Output ringkas:

```text
File kernel/vfs/sys_vfs.c berhasil dibuat (terlihat pada git status).
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[sys_vfs.c]` | `[kernel/vfs/]` | `[Antarmuka syscall VFS]` |

Indikator berhasil:

```text
File kernel/vfs/sys_vfs.c berhasil dibuat setelah perintah diperbaiki.
```

### Langkah 7 — `Menulis unit test host`

Maksud langkah:

```text
Menyediakan unit test yang berjalan di host untuk memvalidasi
VFS/FD/RAMFS tanpa perlu boot kernel/QEMU.
```

Perintah:

```bash
nano tests/m13_vfs_host_test.c
```

Output ringkas:

```text
Untracked files:
        tests/m13_vfs_host_test.c
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m13_vfs_host_test.c]` | `[tests/]` | `[Unit test host VFS/FD/RAMFS]` |

Indikator berhasil:

```text
File tests/m13_vfs_host_test.c berhasil dibuat.
```

### Langkah 8 — `Menulis Makefile.m13`

Maksud langkah:

```text
Menyediakan target build khusus M13 untuk host test dan kompilasi
objek freestanding.
```

Perintah:

```bash
nano Makefile.m13
```

Output ringkas:

```text
Untracked files:
        Makefile.m13
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Makefile.m13]` | `[root repo]` | `[Target build m13-all, clean]` |

Indikator berhasil:

```text
File Makefile.m13 berhasil dibuat.
```

### Langkah 9 — `Build dan menjalankan host test serta objek freestanding`

Maksud langkah:

```text
Membersihkan build lama, lalu menjalankan seluruh target build M13:
kompilasi host test, eksekusi host test, kompilasi objek freestanding
x86_64-elf, dan linking objek menjadi vfs.o.
```

Perintah:

```bash
make -f Makefile.m13 clean
make -f Makefile.m13 m13-all
```

Output ringkas:

```text
rm -rf build/m13
cc -std=c17 -Wall -Wextra -Werror -O2 -Iinclude tests/m13_vfs_host_test.c \
   kernel/vfs/ramfs.c kernel/vfs/fd.c kernel/vfs/sys_vfs.c \
   -o build/m13/m13_vfs_host_test
./build/m13/m13_vfs_host_test | tee build/m13/host-test.log
M13 VFS/FD/RAMFS host tests: PASS

clang -target x86_64-elf -std=c17 -ffreestanding -fno-builtin \
   -fno-stack-protector -fno-pic -mno-red-zone -Wall -Wextra -Werror \
   -O2 -Iinclude -c kernel/vfs/ramfs.c -o build/m13/ramfs.o
clang ... -c kernel/vfs/fd.c -o build/m13/fd.o
clang ... -c kernel/vfs/sys_vfs.c -o build/m13/sys_vfs.o
ld -r -m elf_x86_64 build/m13/ramfs.o build/m13/fd.o build/m13/sys_vfs.o \
   -o build/m13/vfs.o
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m13_vfs_host_test]` | `[build/m13/]` | `[Binary unit test host]` |
| `[host-test.log]` | `[build/m13/]` | `[Log hasil eksekusi host test]` |
| `[ramfs.o, fd.o, sys_vfs.o]` | `[build/m13/]` | `[Objek freestanding x86_64-elf per modul]` |
| `[vfs.o]` | `[build/m13/]` | `[Objek gabungan hasil ld -r]` |

Indikator berhasil:

```text
Host test menghasilkan "M13 VFS/FD/RAMFS host tests: PASS" dan
seluruh objek freestanding berhasil dikompilasi tanpa error/warning
(build memakai -Wall -Wextra -Werror).
```

### Langkah 10 — `Audit objek ELF freestanding`

Maksud langkah:

```text
Memeriksa undefined symbol, header ELF, dan hasil disassembly pada
objek gabungan vfs.o, serta menghasilkan checksum artefak.
```

Perintah:

```bash
nm -u build/m13/vfs.o > build/m13/nm-undefined.txt
readelf -h build/m13/vfs.o > build/m13/readelf-vfs.txt
objdump -dr build/m13/vfs.o > build/m13/objdump-vfs.txt
sha256sum build/m13/ramfs.o build/m13/fd.o build/m13/sys_vfs.o \
   build/m13/vfs.o build/m13/m13_vfs_host_test > build/m13/sha256sums.txt
test ! -s build/m13/nm-undefined.txt
```

Output ringkas:

```text
build/m13/nm-undefined.txt kosong (tidak ada undefined symbol)
test ! -s build/m13/nm-undefined.txt -> lolos (tidak ada output error)
```

Cuplikan `readelf -h build/m13/vfs.o`:

```text
ELF Header:
  Class:                             ELF64
  Data:                              2's complement, little endian
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Number of section headers:         11
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[nm-undefined.txt]` | `[build/m13/]` | `[Bukti tidak ada undefined symbol]` |
| `[readelf-vfs.txt]` | `[build/m13/]` | `[Header ELF vfs.o]` |
| `[objdump-vfs.txt]` | `[build/m13/]` | `[Disassembly vfs.o]` |
| `[sha256sums.txt]` | `[build/m13/]` | `[Checksum seluruh artefak M13]` |

Indikator berhasil:

```text
File nm-undefined.txt kosong, readelf menunjukkan ELF64 REL x86-64
yang valid, dan seluruh artefak berhasil di-checksum.
```

### Langkah 11 — `Commit dan push ke GitHub`

Maksud langkah:

```text
Menyimpan seluruh perubahan M13 ke repository GitHub sebagai
dokumentasi hasil praktikum.
```

Perintah:

```bash
git add Makefile.m13 include/mcs_vfs.h kernel/vfs tests/m13_vfs_host_test.c
git commit -m "M13: implement VFS RAMFS and file descriptor layer"
git push -u origin praktikum-m13-vfs-ramfs
```

Output ringkas:

```text
[praktikum-m13-vfs-ramfs 57d5783] M13: implement VFS RAMFS and file descriptor layer
 6 files changed, 759 insertions(+)

To https://github.com/syifanurzimah/MCSOS.git
 * [new branch]      praktikum-m13-vfs-ramfs -> praktikum-m13-vfs-ramfs
branch 'praktikum-m13-vfs-ramfs' set up to track 'origin/praktikum-m13-vfs-ramfs'.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[commit 57d5783]` | `[GitHub]` | `[Menyimpan hasil praktikum M13]` |
| `[branch praktikum-m13-vfs-ramfs]` | `[GitHub]` | `[Branch remote untuk review/PR]` |

Indikator berhasil:

```text
Perubahan berhasil dipush ke origin/praktikum-m13-vfs-ramfs dan
working tree bersih (git status: nothing to commit, working tree clean).
```

---

## 11. Checkpoint Buildable

Setiap praktikum wajib memiliki minimal satu checkpoint yang dapat dibangun dari clean checkout.

| Checkpoint | Perintah | Expected result | Status |
|---|---|---|---|
| Clean build M13 | `` `make -f Makefile.m13 clean && make -f Makefile.m13 m13-all` `` | `[Host test PASS, objek freestanding & vfs.o berhasil dibangun]` | `[PASS]` |
| Host unit test | `` `./build/m13/m13_vfs_host_test` `` | `[M13 VFS/FD/RAMFS host tests: PASS]` | `[PASS]` |
| Audit undefined symbol | `` `test ! -s build/m13/nm-undefined.txt` `` | `[Tidak ada undefined symbol]` | `[PASS]` |
| QEMU smoke test | `` `(belum ada target)` `` | `[Kernel image dengan VFS boot di QEMU]` | `[NA]` |
| Full kernel image (mcsos.iso) dengan VFS | `` `(belum ada target)` `` | `[VFS terintegrasi ke kernel.elf/mcsos.iso]` | `[NA]` |

Catatan checkpoint:

```text
Tahapan implementasi M13 pada level modul/objek berhasil dijalankan:
host test lulus dengan status PASS, objek freestanding berhasil
dikompilasi, objek digabung menjadi vfs.o tanpa undefined symbol.
Integrasi ke kernel.elf/mcsos.iso dan pengujian pada QEMU belum
dilakukan pada sesi ini sehingga ditandai NA, bukan FAIL.
```

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

Perintah ini memverifikasi bahwa modul M13 dapat dibangun ulang dari kondisi bersih.

```bash
make -f Makefile.m13 clean
make -f Makefile.m13 m13-all
```

Hasil:

```text
Build berhasil tanpa error.
Menghasilkan file:
- build/m13/m13_vfs_host_test
- build/m13/ramfs.o, fd.o, sys_vfs.o
- build/m13/vfs.o
```

Status: `[PASS]`

### 12.2 Static Inspection

Perintah ini memeriksa layout ELF, entry point, section, symbol, dan instruksi kritis sesuai kebutuhan praktikum.

```bash
nm -u build/m13/vfs.o
readelf -h build/m13/vfs.o
objdump -dr build/m13/vfs.o
```

Hasil penting:

```text
- nm -u tidak menghasilkan output (tidak ada undefined symbol).
- readelf -h menunjukkan Class ELF64, Type REL (Relocatable file),
  Machine Advanced Micro Devices X86-64, 11 section headers.
- objdump -dr berhasil menampilkan disassembly objek vfs.o.
```

Status: `[PASS]`

### 12.3 QEMU Smoke Test

```text
BELUM DILAKUKAN. Modul VFS/RAMFS/FD pada M13 belum diintegrasikan ke
kmain()/kernel.elf/mcsos.iso sehingga belum ada target smoke test
QEMU untuk milestone ini.
```

Status: `[NA]`

### 12.4 GDB Debug Evidence

```text
BELUM DILAKUKAN. Karena belum ada kernel image (kernel.elf) yang
memuat modul VFS pada sesi ini, sesi debugging GDB belum relevan
untuk dilakukan pada M13 tahap ini.
```

Status: `[NA]`

### 12.5 Unit Test

```bash
./build/m13/m13_vfs_host_test
```

Hasil:

```text
M13 VFS/FD/RAMFS host tests: PASS
```

Status: `[PASS]`

### 12.6 Stress/Fuzz/Fault Injection Test

Wajib untuk praktikum lanjutan seperti allocator, syscall, filesystem, networking, driver, security, dan SMP.

```bash
BELUM DILAKUKAN
```

Hasil:

```text
Tidak diterapkan pada sesi M13 ini; direncanakan pada tahap integrasi
kernel penuh berikutnya.
```

Status: `[NA]`

### 12.7 Visual Evidence

| Screenshot | Lokasi file | Keterangan |
|---|---|---|
| `[Screenshot build host test PASS]` | `[lampiran]` | `[make -f Makefile.m13 m13-all berhasil]` |
| `[Screenshot nm-undefined.txt kosong]` | `[lampiran]` | `[Tidak ada undefined symbol]` |
| `[Screenshot git push]` | `[lampiran]` | `[Branch berhasil dipush ke GitHub]` |

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| No. | Uji | Expected result | Actual result | Status | Evidence |
|---|---|---|---|---|---|
| 1 | `[Build host test]` | `[Host test berhasil dibangun dan dijalankan]` | `[m13_vfs_host_test berhasil dibuat dan dieksekusi]` | `[PASS]` | `[build/m13/host-test.log]` |
| 2 | `[Hasil host test]` | `[Seluruh assertion VFS/FD/RAMFS lulus]` | `[M13 VFS/FD/RAMFS host tests: PASS]` | `[PASS]` | `[build/m13/host-test.log]` |
| 3 | `[Kompilasi objek freestanding]` | `[ramfs.o, fd.o, sys_vfs.o berhasil dikompilasi]` | `[Ketiga objek berhasil dibuat tanpa warning/error]` | `[PASS]` | `[build/m13/*.o]` |
| 4 | `[Linking objek menjadi vfs.o]` | `[Tidak ada undefined symbol]` | `[nm -u kosong]` | `[PASS]` | `[build/m13/nm-undefined.txt]` |
| 5 | `[Commit dan push]` | `[Perubahan tersimpan di GitHub]` | `[Commit 57d5783 berhasil dipush]` | `[PASS]` | `[origin/praktikum-m13-vfs-ramfs]` |
| 6 | `[QEMU smoke test]` | `[Kernel dengan VFS boot di QEMU]` | `[Belum dilakukan]` | `[NA]` | `[-]` |

### 13.2 Log Penting

```text
M13 VFS/FD/RAMFS host tests: PASS
test ! -s build/m13/nm-undefined.txt   (lolos, file kosong)
```

### 13.3 Artefak Bukti

| Artefak | Path | Ukuran | Fungsi |
|---|---|---|---|
| `m13_vfs_host_test` | `[build/m13/m13_vfs_host_test]` | `[21K]` | `[Binary unit test host]` |
| `host-test.log` | `[build/m13/host-test.log]` | `[34 byte]` | `[Log hasil host test]` |
| `ramfs.o` | `[build/m13/ramfs.o]` | `[3.0K]` | `[Objek freestanding RAMFS]` |
| `fd.o` | `[build/m13/fd.o]` | `[5.1K]` | `[Objek freestanding FD]` |
| `sys_vfs.o` | `[build/m13/sys_vfs.o]` | `[960 byte]` | `[Objek freestanding sys_vfs]` |
| `vfs.o` | `[build/m13/vfs.o]` | `[7.9K]` | `[Objek gabungan hasil ld -r]` |
| `nm-undefined.txt` | `[build/m13/nm-undefined.txt]` | `[0 byte]` | `[Bukti tidak ada undefined symbol]` |
| `readelf-vfs.txt` | `[build/m13/readelf-vfs.txt]` | `[952 byte]` | `[Header ELF vfs.o]` |
| `objdump-vfs.txt` | `[build/m13/objdump-vfs.txt]` | `[80K]` | `[Disassembly vfs.o]` |
| `sha256sums.txt` | `[build/m13/sha256sums.txt]` | `[427 byte]` | `[Checksum seluruh artefak M13]` |

Perintah hash:

```bash
sha256sum build/m13/ramfs.o build/m13/fd.o build/m13/sys_vfs.o \
   build/m13/vfs.o build/m13/m13_vfs_host_test > build/m13/sha256sums.txt
```

Catatan: nilai digest SHA-256 tersimpan pada `build/m13/sha256sums.txt`
dan tidak disalin ulang secara manual pada laporan ini untuk menjaga
laporan tetap sesuai dengan output asli terminal.

---

## 14. Analisis Teknis

### 14.1 Analisis Keberhasilan

```text
Praktikum M13 pada level modul/objek berhasil dilaksanakan dengan
baik. Header VFS, implementasi RAMFS, file descriptor, dan syscall
VFS berhasil dibuat, host test lulus dengan status PASS, objek
freestanding x86_64-elf berhasil dikompilasi dengan flag -Wall -Wextra
-Werror tanpa warning, dan hasil linking (vfs.o) tidak memiliki
undefined symbol.
```

### 14.2 Analisis Kegagalan atau Perbedaan Hasil

```text
Selama proses pengerjaan ditemukan satu kesalahan pengetikan perintah
saat hendak membuat file kernel/vfs/sys_vfs.c: perintah dijalankan
tanpa awalan `nano` sehingga bash mencoba mengeksekusi nama file
tersebut sebagai program dan menghasilkan error "No such file or
directory". Kesalahan ini bersifat operasional (human error pada
command line), bukan kegagalan build/kode, dan langsung diperbaiki
dengan mengulang perintah menggunakan `nano kernel/vfs/sys_vfs.c`.
```

### 14.3 Perbandingan dengan Teori

| Konsep teori | Implementasi praktikum | Sesuai/tidak sesuai | Penjelasan |
|---|---|---|---|
| `[Abstraksi VFS generik]` | `[sys_vfs.c sebagai titik masuk, fd.c sebagai lapisan mediasi]` | `[sesuai]` | `[Struktur modul memisahkan antarmuka dari backend RAMFS]` |
| `[File descriptor sebagai indeks ke objek file]` | `[fd.c mengelola tabel FD]` | `[sesuai]` | `[Modul terpisah khusus untuk manajemen FD]` |
| `[Objek relocatable ELF64]` | `[vfs.o hasil ld -r]` | `[sesuai]` | `[readelf menunjukkan Type: REL (Relocatable file)]` |

### 14.4 Kompleksitas dan Kinerja

| Aspek | Estimasi/hasil | Bukti | Catatan |
|---|---|---|---|
| Kompleksitas algoritma | `[TIDAK DIUKUR]` | `[-]` | `[Belum dianalisis secara formal pada M13 tahap ini]` |
| Waktu build M13 | `[kurang dari 1 menit]` | `[output make]` | `[Build host test + freestanding berjalan cepat]` |
| Waktu eksekusi host test | `[TIDAK DIUKUR]` | `[host-test.log]` | `[Hanya status PASS yang tercatat]` |
| Penggunaan memori | `[TIDAK DIUKUR]` | `[-]` | `[Belum relevan karena belum berjalan di kernel/QEMU]` |
| Latensi/throughput | `[TIDAK DIUKUR]` | `[-]` | `[Belum ada di M13 tahap ini]` |

---

## 15. Debugging dan Failure Modes

### 15.1 Failure Modes yang Ditemukan

| Failure mode | Gejala | Penyebab sementara | Bukti | Perbaikan |
|---|---|---|---|---|
| `[Salah ketik perintah pembuatan file]` | `[-bash: kernel/vfs/sys_vfs.c: No such file or directory]` | `[Lupa mengetik "nano" sebelum nama file]` | `[terminal]` | `[Mengulang perintah dengan "nano kernel/vfs/sys_vfs.c"]` |

### 15.2 Failure Modes yang Diantisipasi

| Failure mode | Deteksi | Dampak | Mitigasi |
|---|---|---|---|
| `[undefined symbol pada vfs.o]` | `[nm -u]` | `[Kernel gagal link saat integrasi penuh]` | `[Audit nm -u dan test ! -s nm-undefined.txt pada setiap build]` |
| `[Node RAMFS/FD invalid]` | `[host test]` | `[Perilaku tidak terdefinisi saat operasi file]` | `[Validasi pada host test sebelum integrasi kernel]` |

### 15.3 Triage yang Dilakukan

```text
1. Membuat branch dan struktur direktori kerja M13.
2. Menulis header dan implementasi VFS/RAMFS/FD/sys_vfs.
3. Memperbaiki kesalahan ketik perintah pembuatan sys_vfs.c.
4. Menjalankan make -f Makefile.m13 clean && make -f Makefile.m13 m13-all.
5. Memeriksa hasil host test (PASS).
6. Mengaudit objek freestanding menggunakan nm, readelf, objdump.
7. Menghasilkan checksum artefak dengan sha256sum.
8. Melakukan commit dan push ke GitHub.
```

### 15.4 Panic Path

```text
Belum ada uji panic path pada M13 tahap ini karena modul VFS/RAMFS/FD
belum diintegrasikan ke kmain()/kernel.elf. Fokus M13 pada sesi ini
adalah validasi host test dan audit objek freestanding sebelum tahap
integrasi kernel dilakukan pada iterasi berikutnya.
```

---

## 16. Prosedur Rollback

Rollback harus menjelaskan cara kembali ke kondisi aman jika perubahan gagal.

| Skenario rollback | Perintah | Data yang harus diselamatkan | Status |
|---|---|---|---|
| Kembali ke commit awal (sebelum M13) | `` `git checkout 673ea92` `` | `[Log build dan evidence M13]` | `[belum diuji]` |
| Revert implementasi M13 | `` `git revert 57d5783` `` | `[Artefak build/m13 dan log pengujian]` | `[belum diuji]` |
| Bersihkan artefak build M13 | `` `make -f Makefile.m13 clean` `` | `[Tidak ada, karena hanya menghapus hasil build/m13]` | `[teruji]` |
| Bangun ulang objek M13 | `` `make -f Makefile.m13 m13-all` `` | `[Artefak lama jika masih diperlukan]` | `[teruji]` |

Catatan rollback:

```text
Pada praktikum ini rollback penuh menggunakan Git tidak dilakukan
karena implementasi berjalan sesuai target host test (PASS) dan audit
ELF (tidak ada undefined symbol). Mekanisme rollback melalui Git
telah disiapkan apabila di kemudian hari diperlukan. Perintah `make
-f Makefile.m13 clean` telah diuji dan berhasil menghapus artefak
build/m13 tanpa memengaruhi source code.
```

---

## 17. Keamanan dan Reliability

### 17.1 Risiko Keamanan

| Risiko | Boundary | Dampak | Mitigasi | Evidence |
|---|---|---|---|---|
| `[Undefined symbol pada objek vfs.o]` | `[linker (ld -r)]` | `[Kernel gagal link saat integrasi]` | `[Audit menggunakan nm -u]` | `[build/m13/nm-undefined.txt]` |
| `[Input tidak valid pada antarmuka sys_vfs]` | `[Pemanggil → sys_vfs.c]` | `[Perilaku tidak terdefinisi pada FD/RAMFS]` | `[Validasi parameter direncanakan pada sys_vfs.c]` | `[kernel/vfs/sys_vfs.c]` |
| `[Kesalahan layout objek ELF relocatable]` | `[Kompilasi & linking]` | `[Objek gagal digunakan pada tahap integrasi]` | `[Verifikasi menggunakan readelf dan objdump]` | `[build/m13/readelf-vfs.txt, objdump-vfs.txt]` |

### 17.2 Reliability dan Data Integrity

| Risiko reliability | Dampak | Deteksi | Mitigasi |
|---|---|---|---|
| `[Build freestanding gagal]` | `[Objek vfs.o tidak tersedia]` | `[Output make -f Makefile.m13]` | `[Memperbaiki source dan build ulang]` |
| `[Host test gagal]` | `[Modul VFS/FD/RAMFS dianggap belum siap]` | `[build/m13/host-test.log]` | `[Perbaikan logika sebelum lanjut ke audit ELF]` |
| `[RAMFS kehilangan data saat reboot]` | `[Data tidak persisten]` | `[Desain (bagian 9.2)]` | `[Diketahui sebagai batasan RAMFS in-memory pada M13]` |

### 17.3 Negative Test

| Negative test | Input buruk | Expected result | Actual result | Status |
|---|---|---|---|---|
| `[Audit undefined symbol]` | `[nm -u build/m13/vfs.o]` | `[Tidak ada undefined symbol]` | `[File nm-undefined.txt kosong]` | `[PASS]` |
| `[Build dengan warning sebagai error]` | `[-Wall -Wextra -Werror]` | `[Build gagal jika ada warning]` | `[Build berhasil tanpa warning]` | `[PASS]` |
| `[Validasi hasil host test]` | `[Menjalankan m13_vfs_host_test]` | `[Status PASS]` | `[M13 VFS/FD/RAMFS host tests: PASS]` | `[PASS]` |

---

## 18. Pembagian Kerja Kelompok

Isi bagian ini hanya jika praktikum dikerjakan berkelompok. Untuk pengerjaan individu, tulis "Tidak berlaku".

```text
Tidak berlaku — praktikum M13 ini dikerjakan secara individu.
```

### 18.1 Mekanisme Koordinasi

```text
Tidak berlaku (praktikum individu).
```

### 18.2 Evaluasi Kontribusi

| Anggota | Persentase kontribusi yang disepakati | Bukti | Catatan |
|---|---:|---|---|
| `[Syifa Nurzimah]` | `[100%]` | `[Repository Git, commit history, artefak build/m13]` | `[praktikum individu]` |

---

## 19. Kriteria Lulus Praktikum

Bagian ini wajib diisi. Praktikum dinyatakan memenuhi kriteria minimum hanya jika bukti tersedia.

| Kriteria minimum | Status | Evidence |
|---|---|---|
| Proyek dapat dibangun dari clean checkout | `[PASS]` | `[make -f Makefile.m13 clean && make -f Makefile.m13 m13-all]` |
| Perintah build terdokumentasi | `[PASS]` | `[bagian 10]` |
| QEMU boot atau test target berjalan deterministik | `[NA]` | `[Belum diintegrasikan ke kernel/QEMU pada M13 tahap ini]` |
| Semua unit test/praktikum test relevan lulus | `[PASS]` | `[build/m13/host-test.log: PASS]` |
| Log serial disimpan | `[NA]` | `[Tidak relevan, belum ada kernel image pada M13 tahap ini]` |
| Panic path terbaca atau dijelaskan jika belum relevan | `[NA]` | `[bagian 15.4]` |
| Tidak ada warning kritis pada build | `[PASS]` | `[build dengan -Wall -Wextra -Werror]` |
| Perubahan Git terkomit | `[PASS]` | `[Commit 57d5783]` |
| Desain dan failure mode dijelaskan | `[PASS]` | `[bagian 9 dan 15]` |
| Laporan berisi screenshot/log yang cukup | `[PASS]` | `[lampiran]` |

Kriteria tambahan untuk praktikum lanjutan:

| Kriteria lanjutan | Status | Evidence |
|---|---|---|
| Static analysis dijalankan | `[PASS]` | `[nm, readelf, objdump]` |
| Stress test dijalankan | `[NA]` | `[Tidak menjadi fokus M13 tahap ini]` |
| Fuzzing atau malformed-input test dijalankan | `[NA]` | `[Tidak menjadi fokus M13 tahap ini]` |
| Fault injection dijalankan | `[NA]` | `[Belum ada mekanisme panic test M13]` |
| Disassembly/readelf evidence tersedia | `[PASS]` | `[build/m13/objdump-vfs.txt, readelf-vfs.txt]` |
| Review keamanan dilakukan | `[PASS]` | `[bagian 17]` |
| Rollback diuji | `[NA]` | `[Belum dilakukan; hanya make clean yang teruji]` |

---

## 20. Readiness Review

Pilih satu status dengan alasan berbasis bukti.

| Status | Definisi | Pilihan |
|---|---|---|
| Belum siap uji | Build/test belum stabil atau bukti belum cukup | `[ ]` |
| Siap uji QEMU | Build bersih, QEMU/test target berjalan, log tersedia | `[ ]` |
| Siap demonstrasi praktikum | Siap ditunjukkan di kelas dengan bukti uji, failure mode, dan rollback | `[ ]` |
| Kandidat siap pakai terbatas | Hanya untuk penggunaan terbatas setelah test, security review, dokumentasi, dan known issue tersedia | `[ x ]` |

Alasan readiness:

```text
Berdasarkan hasil build host test (PASS), kompilasi objek freestanding
x86_64-elf tanpa warning, serta audit ELF (tidak ada undefined
symbol), implementasi modul VFS/RAMFS/FD M13 telah memenuhi
pemeriksaan mekanis pada level modul/objek. Namun karena modul ini
belum diintegrasikan ke kmain()/kernel.elf, belum ada image ISO, dan
belum diuji pada QEMU maupun GDB, praktikum M13 pada tahap ini
dinyatakan sebagai kandidat siap pakai terbatas — siap digunakan
sebagai modul yang telah tervalidasi secara host/objek, namun belum
siap untuk demonstrasi kernel penuh.
```

Known issues:

| No. | Issue | Dampak | Workaround | Target perbaikan |
|---|---|---|---|---|
| 1 | `[VFS/RAMFS/FD belum terhubung ke kmain()/kernel.elf]` | `[Belum bisa diuji end-to-end pada QEMU]` | `[Gunakan host test sebagai validasi sementara]` | `[Tahap integrasi M13 berikutnya / M14]` |
| 2 | `[Belum ada folder evidence/M13 resmi]` | `[Dokumentasi bukti belum terpusat seperti evidence/M12]` | `[Gunakan build/m13 sebagai bukti sementara]` | `[Tahap pengumpulan evidence berikutnya]` |
| 3 | `[Locking/mutex M12 belum diintegrasikan ke VFS/FD]` | `[Belum ada jaminan concurrency-safety]` | `[Belum ada, karena masih single-path pada host test]` | `[Tahap integrasi sinkronisasi berikutnya]` |

Keputusan akhir:

```text
Berdasarkan hasil pengujian dan evidence yang diperoleh, praktikum
M13 (VFS/RAMFS/FD) dinyatakan sebagai kandidat siap pakai terbatas.
Proses build host test, kompilasi objek freestanding, audit ELF, dan
commit/push ke GitHub berhasil dijalankan, namun integrasi ke kernel
penuh dan pengujian QEMU/GDB belum dilakukan pada sesi ini.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Indikator nilai penuh | Nilai |
|---|---:|---|---:|
| Kebenaran fungsional | 30 | Implementasi memenuhi target praktikum, build/test lulus, output sesuai expected result | `[25]` |
| Kualitas desain dan invariants | 20 | Desain jelas, kontrak antarmuka eksplisit, invariants/ownership/locking terdokumentasi | `[16]` |
| Pengujian dan bukti | 20 | Unit/integration/QEMU/static/fuzz/stress evidence memadai sesuai tingkat praktikum | `[13]` |
| Debugging dan failure analysis | 10 | Failure mode, triage, panic/log, dan rollback dianalisis | `[7]` |
| Keamanan dan robustness | 10 | Boundary, input validation, privilege, memory safety, dan negative tests dibahas | `[6]` |
| Dokumentasi dan laporan | 10 | Laporan rapi, lengkap, dapat direproduksi, memakai referensi yang layak | `[9]` |
| **Total** | **100** |  | `[76]` |

Catatan penilai:

```text
[Diisi dosen/asisten. Nilai pada kolom "Nilai" di atas merupakan
estimasi mandiri (self-assessment) berdasarkan bukti yang tersedia
pada sesi terminal M13 ini, mengingat integrasi kernel penuh, uji
QEMU, dan GDB belum dilakukan.]
```

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Praktikum M13 berhasil menambahkan header VFS, implementasi RAMFS,
lapisan file descriptor, dan antarmuka syscall VFS (sys_vfs). Host
unit test berhasil dijalankan dengan status PASS, objek freestanding
x86_64-elf berhasil dikompilasi tanpa warning, dan hasil linking
(vfs.o) tidak memiliki undefined symbol. Seluruh perubahan berhasil
dikomit dan dipush ke GitHub pada commit 57d5783.
```

### 22.2 Yang Belum Berhasil

```text
Integrasi modul VFS/RAMFS/FD ke kmain()/kernel.elf, pembuatan image
ISO dengan VFS, pengujian smoke test pada QEMU, debugging
menggunakan GDB, integrasi dengan primitif sinkronisasi M12, serta
pengumpulan evidence resmi ke folder evidence/M13 belum dilakukan
pada sesi ini.
```

### 22.3 Rencana Perbaikan

```text
Tahap selanjutnya adalah mengintegrasikan modul VFS/RAMFS/FD ke
kmain()/kernel.elf, menjalankan smoke test pada QEMU, melakukan
debugging menggunakan GDB, menghubungkan lapisan locking dari M12 ke
VFS/FD, serta mengumpulkan evidence resmi ke folder evidence/M13
sebelum melanjutkan ke milestone berikutnya.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
git log --oneline -5
```

### Lampiran B — Diff Ringkas

```diff
git diff --stat
```

### Lampiran C — Log Build Lengkap

```text
make -f Makefile.m13 clean
make -f Makefile.m13 m13-all
(lihat bagian 10, Langkah 9 untuk output lengkap)
```

### Lampiran D — Log Host Test

```text
build/m13/host-test.log
```

### Lampiran E — Output Readelf/Objdump/nm

```text
build/m13/readelf-vfs.txt
build/m13/objdump-vfs.txt
build/m13/nm-undefined.txt
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[Screenshot Terminal]` | `[Build dan host test PASS]` |
| 2 | `[Screenshot GitHub]` | `[Repository setelah push branch praktikum-m13-vfs-ramfs]` |

### Lampiran G — Bukti Tambahan

```text
build/m13/
```

---

## 24. Daftar Referensi

Gunakan format IEEE. Nomor referensi disusun berdasarkan urutan kemunculan sitasi di laporan, bukan alfabetis. Contoh format:

```text
[1] R. H. Arpaci-Dusseau and A. C. Arpaci-Dusseau, Operating Systems: Three Easy Pieces. Available: https://pages.cs.wisc.edu/~remzi/OSTEP/
[2] Intel Corporation, Intel® 64 and IA-32 Architectures Software Developer's Manual. Available: https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
[3] QEMU Project Documentation. Available: https://www.qemu.org/docs/master/
[4] LLVM Project Documentation. Available: https://clang.llvm.org/docs/
[5] GNU Binutils Documentation. Available: https://sourceware.org/binutils/docs/
```

Referensi yang benar-benar dipakai dalam laporan:

```text
[1] LLVM Project Documentation (Clang, target freestanding). Available: https://clang.llvm.org/docs/
[2] GNU Binutils Documentation (nm, readelf, objdump, ld). Available: https://sourceware.org/binutils/docs/
[3] Institute of Electrical and Electronics Engineers, IEEE Std 1003.1 (POSIX), konsep antarmuka file dan file descriptor.
```

---

## 25. Checklist Final Sebelum Pengumpulan

| Checklist | Status |
|---|---|
| Semua placeholder `[isi ...]` sudah diganti | `[Ya]` |
| Metadata laporan lengkap | `[Ya]` |
| Commit awal dan akhir dicatat | `[Ya]` |
| Perintah build dan test dapat dijalankan ulang | `[Ya]` |
| Log build dilampirkan | `[Ya]` |
| Log QEMU/test dilampirkan | `[Tidak — QEMU belum dijalankan pada M13 tahap ini]` |
| Artefak penting diberi hash | `[Ya, via build/m13/sha256sums.txt]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya, sebagai self-assessment]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya/kami mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
Saya mengumpulkan laporan ini bersama artefak pendukung pada commit:

57d5783

Status akhir yang diklaim:

Kandidat siap pakai terbatas
```

Status akhir yang diklaim:

```text
[Kandidat siap pakai terbatas]
```

Ringkasan satu paragraf:

```text
[Praktikum M13 berhasil menambahkan lapisan Virtual File System
(VFS), backend RAMFS in-memory, lapisan file descriptor, dan
antarmuka syscall VFS pada MCSOS. Host unit test lulus dengan status
PASS, objek freestanding x86_64-elf berhasil dikompilasi tanpa
warning, dan hasil linking (vfs.o) tidak memiliki undefined symbol.
Seluruh perubahan telah dikomit dan dipush ke GitHub pada commit
57d5783. Integrasi ke kernel.elf, image ISO, QEMU, dan GDB belum
dilakukan pada sesi ini sehingga status readiness dinyatakan sebagai
kandidat siap pakai terbatas.]
```
