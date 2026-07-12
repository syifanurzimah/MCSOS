 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M14

**Nama file laporan:** `laporan_praktikum_m14_25832074009.md`  
**Nama sistem operasi:** MCSOS versi 260502  
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset  
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.  
**Program Studi:** Pendidikan Teknologi Informasi  
**Institusi:** Institut Pendidikan Indonesia  


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M14]` |
| Judul praktikum | `[Block Device Layer: Registry Device Blok, RAM Block Device, dan Buffer Cache (Clock/Second-Chance)]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[isi jika kelompok]` |
| Anggota kelompok | `[nama, NIM, peran ringkas]` |
| Tanggal praktikum | `[2026-07-07]` |
| Tanggal pengumpulan | `[2026-07-08]` |
| Repository | `[https://github.com/syifanurzimah/MCSOS]` |
| Branch | `[praktikum-m14-block-device]` |
| Commit awal | `` `[HEAD branch praktikum-m13-vfs-ramfs sebelum branching; hash tidak tercatat eksplisit pada transkrip terminal]` `` |
| Commit akhir | `` `[a64ca8e]` `` |
| Status readiness yang diklaim | `[Siap uji (host test & freestanding build & audit objdump/nm); QEMU/GDB runtime belum dilakukan pada sesi ini]` |

---

## 1. Sampul

# Laporan Praktikum `M14`  
## `Block Device Layer: Registry Device Blok, RAM Block Device, dan Buffer Cache (Clock/Second-Chance)`

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
Clang, GNU Binutils (readelf, objdump, nm), GNU Make, Git, serta AI
Assistant (ChatGPT/Claude) untuk membantu menjelaskan konsep block
device layer dan buffer cache, memperbaiki error build (duplikasi
fungsi bcache pada ramblk.c, ketidaksesuaian anggota struct
mcsos_blk_device/mcsos_ramblk, ketidakcocokan tipe pointer fungsi
flush), menyusun script preflight, dan membantu penyusunan laporan.

Seluruh kode yang dihasilkan diverifikasi kembali dengan proses build
host-test (tests/host/test_m14_block.c), build freestanding x86_64
(make all dan make m14-freestanding), serta audit nm -u dan
objdump/readelf terhadap objek m14_block_layer.o untuk memastikan
tidak ada unresolved symbol pada implementasi registry device,
RAM block device, dan buffer cache clock/second-chance.
```

---

## 3. Tujuan Praktikum

1. `Membangun kernel MCSOS milestone M14 dengan menambahkan Block Device Layer sebagai lanjutan dari VFS/ramfs pada milestone M13.`
2. `Mengimplementasikan registry device blok (mcsos_blk_register, mcsos_blk_get, mcsos_blk_count) beserta operasi baca/tulis/flush generik (mcsos_blk_read, mcsos_blk_write, mcsos_blk_flush) yang tervalidasi terhadap rentang LBA (mcsos_blk_validate_range).`
3. `Mengimplementasikan RAM block device (mcsos_ramblk_init) sebagai driver device blok berbasis memori, serta buffer cache (mcsos_bcache_init/read/write/flush_all) dengan kebijakan penggantian clock/second-chance (clock_hand) dan mekanisme dirty-bit sebelum write-back.`
4. `Memvalidasi implementasi menggunakan unit test host (tests/host/test_m14_block.c) dan target Makefile m14-host-test/m14-freestanding/m14-audit yang memeriksa unresolved symbol (nm -u) serta struktur ELF (readelf, objdump) pada hasil kompilasi freestanding x86_64.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Mampu membangun kernel M14 dengan modul block device layer (block.c, ramblk.c, bcache.c, block.h)]` | `[build berhasil, kernel.elf, block.o/ramblk.o/bcache.o terkompilasi dan terlink pada target make all]` |
| `[Mampu melakukan audit objdump/nm untuk memastikan tidak ada unresolved symbol pada block layer]` | `[nm -u build/m14/m14_block_layer.o menghasilkan file kosong (m14_nm_undefined.txt)]` |
| `[Mampu menulis dan menjalankan unit test host untuk logika block device dan buffer cache tanpa hardware nyata]` | `[./build/m14/test_m14_block mencetak "M14 host tests PASS"]` |
| `[Mampu men-debug error build akibat duplikasi definisi fungsi (multiple definition) dan ketidaksesuaian anggota struct pada implementasi ramblk.c]` | `[perbaikan kode ramblk.c melalui nano hingga build/m14/ramblk.o dan build/normal/kernel/block/ramblk.o berhasil dikompilasi]` |

---

## 5. Peta Milestone MCSOS

| Milestone | Fokus | Status dalam laporan |
|---|---|---|
| M0 | Requirements, governance, baseline arsitektur | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M1 | Toolchain reproducible, Git, QEMU, GDB, metadata build | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M2 | Boot image, kernel ELF64, early console | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M3 | Panic path, linker map, GDB, observability awal | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M4 | Trap, exception, interrupt, timer | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M5 | External interrupt bring-up (PIC, PIT, IRQ0) | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M6 | Physical Memory Manager (PMM) | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M7 | Virtual Memory Manager (VMM), page table, invalidasi TLB | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M8 | Kernel heap | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M9 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M10 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M11 | ELF user loader | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M12 | Synchronization lanjutan | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M13 | VFS, file descriptor, ramfs | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M14 | Block device layer, RAM block device, buffer cache | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M15 | SMP, scalability, lock stress, NUMA-aware preparation | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Catatan: Riwayat branch aktual pada repository (`praktikum-m4-idt-exception-path`, `praktikum-m8-kernel-heap`, `praktikum-m9-scheduler`, `praktikum/m10-syscall-abi`, `praktikum/m11-elf-user-loader`, `praktikum/m12-sync`, `praktikum-m13-vfs-ramfs`, `praktikum/m5-timer-irq`, `praktikum/m6-pmm`, `praktikum/m7-vmm`) menunjukkan bahwa milestone sebelumnya sudah pernah dikerjakan pada branch masing-masing. Praktikum M14 ini dibuat pada branch baru `praktikum-m14-block-device` yang dicabangkan dari HEAD branch `praktikum-m13-vfs-ramfs`.

Batas cakupan praktikum:

```text
Praktikum M14 berfokus pada implementasi Block Device Layer: struktur
data device blok generik (mcsos_blk_device_t, mcsos_blk_ops_t),
registry device blok dengan batas maksimum 8 device
(MCSOS_BLK_MAX_DEVICES), operasi baca/tulis/flush generik yang
tervalidasi terhadap rentang LBA, driver RAM block device
(mcsos_ramblk_init) sebagai device blok berbasis memori, serta buffer
cache (mcsos_bcache_t) dengan kebijakan penggantian clock/
second-chance dan mekanisme dirty-bit sebelum write-back ke device.

Praktikum ini melanjutkan VFS/ramfs dari M13 dan belum membahas
filesystem persisten (mcsfs/ext2-like) yang akan dikembangkan pada
milestone berikutnya. Pengujian runtime penuh melalui QEMU/GDB untuk
build M14 belum dilakukan pada sesi ini; verifikasi dilakukan melalui
unit test host dan audit statis (nm/readelf/objdump) terhadap objek
freestanding x86_64 (lihat bagian 14.2 dan 20).
```

---

## 6. Dasar Teori Ringkas

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Pada praktikum M14, konsep utama yang dipelajari adalah Block Device
Layer, yaitu bagaimana kernel menyediakan abstraksi seragam
(mcsos_blk_device_t) di atas beragam media penyimpanan blok melalui
tabel fungsi (mcsos_blk_ops_t: read/write/flush). Setiap operasi I/O
harus divalidasi terhadap batas LBA (Logical Block Address) sebelum
diteruskan ke driver, agar operasi baca/tulis di luar kapasitas device
tidak menimbulkan akses memori yang tidak valid. Di atas device blok,
buffer cache (mcsos_bcache_t) menyimpan salinan blok yang baru diakses
di memori agar mengurangi jumlah I/O fisik, menggunakan kebijakan
penggantian clock/second-chance (clock_hand berputar pada entri cache)
serta menandai entri sebagai dirty ketika ditulis, sehingga entri
dirty wajib di-flush (write-back) ke device sebelum digantikan atau
ketika mcsos_bcache_flush_all() dipanggil eksplisit.
```

### 6.2 Konsep Arsitektur Block Layer yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[Registry device blok statis (array g_blk_devices[MCSOS_BLK_MAX_DEVICES])]` | `[Menyimpan pointer device yang terdaftar melalui mcsos_blk_register()]` | `[source code block.c: g_blk_devices, g_blk_count]` |
| `[Validasi rentang LBA sebelum I/O]` | `[mcsos_blk_validate_range() memeriksa lba >= block_count dan overflow count]` | `[source code block.c: mcsos_blk_validate_range]` |
| `[RAM block device sebagai driver referensi]` | `[mcsos_ramblk_init() memetakan buffer memori sebagai media blok tanpa hardware nyata]` | `[source code ramblk.c: mcsos_ramblk_init, mcsos_ramblk_read/write/flush]` |
| `[Buffer cache dengan algoritma clock/second-chance]` | `[mcsos_bcache_select_victim() memilih entri kosong terlebih dahulu, lalu memutar clock_hand pada entri terisi]` | `[source code bcache.c: mcsos_bcache_select_victim]` |
| `[Dirty bit dan write-back]` | `[Entri yang ditulis melalui mcsos_bcache_write() ditandai dirty=1 dan baru ditulis ke device saat digantikan atau saat flush_all]` | `[source code bcache.c: mcsos_bcache_flush_entry, mcsos_bcache_flush_all]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding untuk kernel (target x86_64-unknown-none-elf); C17 host-mode untuk unit test]` |
| Runtime | `[tanpa hosted libc pada mode kernel; hanya stddef.h dan stdint.h yang dipakai pada block.h]` |
| ABI | `[x86_64 System V ABI]` |
| Mekanisme testability | `[Kode block.c/ramblk.c/bcache.c yang sama dikompilasi dua kali: sekali sebagai host test biasa (clang -std=c17, tanpa target freestanding) untuk dijalankan langsung, dan sekali sebagai objek freestanding (--target=x86_64-unknown-none-elf -ffreestanding) yang diaudit dengan nm/readelf/objdump]` |
| Risiko undefined behavior | `[Duplikasi definisi fungsi bcache akibat kesalahan salin-tempel ke ramblk.c, penulisan langsung ke anggota struct yang tidak ada pada header (priv, data, bytes, read, write), ketidakcocokan tipe pointer fungsi flush terhadap mcsos_blk_rw_fn]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[Operating Systems: Three Easy Pieces]` | `[Bab I/O Devices dan Disk/Buffer Cache]` | `[Referensi konsep registry device blok, validasi LBA, dan kebijakan penggantian cache]` |
| `[2]` | `[Clang/LLVM Documentation]` | `[--target=x86_64-unknown-none-elf, -ffreestanding, -Wall -Wextra -Werror]` | `[Menyelesaikan error build freestanding terkait tipe pointer fungsi dan anggota struct]` |
| `[3]` | `[GNU Binutils Documentation]` | `[nm -u, readelf -h, objdump -dr]` | `[Audit symbol dan struktur ELF pada build/m14/m14_block_layer.o]` |
| `[4]` | `[Dokumentasi Git]` | `[git switch -c, git status --short, git add, git commit, git push]` | `[Pengelolaan perubahan kode M14 pada branch praktikum-m14-block-device]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows x64]` |
| Lingkungan build | `[WSL 2 Ubuntu 26.04 LTS (codename resolute), kernel Linux 6.18.33.2-microsoft-standard-WSL2]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-unknown-none-elf]` |
| Emulator | `[QEMU (tersedia, belum dijalankan pada sesi M14 ini)]` |
| Debugger | `[belum digunakan pada sesi M14 ini]` |
| Build system | `[GNU Make]` |
| Bahasa utama | `[C17 Freestanding (kernel) dan C17 host-mode (unit test)]` |
| Assembly | `[tidak ada assembly baru pada modul block layer M14]` |

### 7.2 Versi Toolchain

Tempel output versi toolchain berikut. Jalankan dari clean shell WSL.

```bash
{ uname -a; lsb_release -a 2>/dev/null || cat /etc/os-release; } | tee artifacts/m14/host_info.txt

{ clang --version; ld --version | head -n 1; nm --version | head -n 1; \
  readelf --version | head -n 1; objdump --version | head -n 1; \
  make --version | head -n 1; qemu-system-x86_64 --version; } | tee artifacts/m14/tool_versions.txt
```

Output:

```text
Linux WIN-E2QNIIEGDH4 6.18.33.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun 18 21:54:43 UTC 2026 x86_64 GNU/Linux
Distributor ID: Ubuntu
Description:    Ubuntu 26.04 LTS
Release:        26.04
Codename:       resolute

Ubuntu clang version 21.1.8 (6ubuntu1)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/lib/llvm-21/bin
GNU ld (GNU Binutils for Ubuntu) 2.46
GNU nm (GNU Binutils for Ubuntu) 2.46
GNU readelf (GNU Binutils for Ubuntu) 2.46
GNU objdump (GNU Binutils for Ubuntu) 2.46
GNU Make 4.4.1
QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
Copyright (c) 2003-2025 Fabrice Bellard and the QEMU Project developers
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[~/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[praktikum-m14-block-device]` |
| Commit hash awal | `` `[HEAD praktikum-m13-vfs-ramfs sebelum branching, hash tidak tercatat eksplisit]` `` |
| Commit hash akhir | `` `[a64ca8e]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

```text
mcsos/
├── include/
│   └── mcsos/
│       └── block.h
├── kernel/
│   └── block/
│       ├── block.c
│       ├── ramblk.c
│       └── bcache.c
├── tests/
│   └── host/
│       └── test_m14_block.c
├── scripts/
│   └── m14_preflight.sh
├── artifacts/
│   └── m14/
│       ├── host_info.txt
│       ├── tool_versions.txt
│       ├── preflight.log
│       ├── git_status_before_m14.txt
│       └── m14_make_all.log
├── build/
│   └── m14/
│       ├── block.o
│       ├── ramblk.o
│       ├── bcache.o
│       ├── m14_block_layer.o
│       ├── m14_nm_undefined.txt
│       ├── m14_readelf.txt
│       ├── m14_objdump.txt
│       └── test_m14_block
├── Makefile
└── linker.ld
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[include/mcsos/block.h]` | `[baru]` | `[Header definisi tipe device blok, ops, RAM block device, dan buffer cache serta deklarasi fungsi]` | `[tinggi]` |
| `[kernel/block/block.c]` | `[baru]` | `[Implementasi registry device blok, validasi rentang LBA, dan operasi baca/tulis/flush generik]` | `[tinggi]` |
| `[kernel/block/ramblk.c]` | `[baru]` | `[Implementasi RAM block device (driver berbasis memori)]` | `[tinggi]` |
| `[kernel/block/bcache.c]` | `[baru]` | `[Implementasi buffer cache dengan kebijakan penggantian clock/second-chance]` | `[tinggi]` |
| `[tests/host/test_m14_block.c]` | `[baru]` | `[Unit test host untuk registry device blok, RAM block device, dan buffer cache]` | `[rendah]` |
| `[scripts/m14_preflight.sh]` | `[baru]` | `[Script preflight: memeriksa toolchain, direktori, dokumen panduan, dan status git sebelum mulai coding M14]` | `[rendah]` |
| `[Makefile]` | `[ubah]` | `[Menambah sumber kompilasi block.c/ramblk.c/bcache.c ke build normal serta target m14-host-test, m14-freestanding, dan m14-audit]` | `[sedang]` |
| `[artifacts/m14/*.txt, *.log]` | `[baru]` | `[Artefak bukti host info, versi tool, hasil preflight, status git sebelum M14, dan log make all]` | `[rendah]` |

### 8.3 Ringkasan Diff

```text
git status --short
git add Makefile include/mcsos/block.h kernel/block scripts/m14_preflight.sh tests/host artifacts
git commit -m "praktikum M14: add block device layer and buffer cache"
10 files changed, 634 insertions(+)
 create mode 100644 artifacts/m14/git_status_before_m14.txt
 create mode 100644 artifacts/m14/host_info.txt
 create mode 100644 artifacts/m14/tool_versions.txt
 create mode 100644 include/mcsos/block.h
 create mode 100644 kernel/block/bcache.c
 create mode 100644 kernel/block/block.c
 create mode 100644 kernel/block/ramblk.c
 create mode 100755 scripts/m14_preflight.sh
 create mode 100644 tests/host/test_m14_block.c
```

---

## 9. Desain Teknis

### 9.1 Masalah yang Diselesaikan

```text
Kernel pada milestone sebelumnya (M13) sudah memiliki VFS dan ramfs,
namun kernel belum memiliki abstraksi device blok yang seragam sebagai
lapisan di bawah filesystem. Pada praktikum M14 ditambahkan Block
Device Layer yang menyediakan registry device blok generik, driver RAM
block device sebagai referensi tanpa hardware nyata, serta buffer
cache dengan kebijakan penggantian clock/second-chance untuk mengurangi
jumlah I/O fisik ke device, sebagai pondasi bagi filesystem persisten
(mcsfs/ext2-like) pada milestone berikutnya (M12 pada peta roadmap
16-milestone).
```

### 9.2 Keputusan Desain

| Keputusan | Alternatif yang dipertimbangkan | Alasan memilih | Konsekuensi |
|---|---|---|---|
| `[Registry device blok statis berbasis array (MCSOS_BLK_MAX_DEVICES = 8)]` | `[Linked list device dinamis]` | `[Lebih sederhana dan mudah diverifikasi tanpa alokator memori dinamis pada tahap ini]` | `[Jumlah device blok dibatasi maksimum 8; mengembalikan MCSOS_BLK_EFULL jika penuh]` |
| `[Validasi block_size harus power-of-two dan minimal MCSOS_BLK_DEFAULT_SECTOR_SIZE (512)]` | `[Menerima sembarang ukuran blok]` | `[Menjamin operasi aritmatika LBA/offset sederhana dan konsisten dengan sektor disk nyata]` | `[Device dengan ukuran blok tidak baku ditolak saat registrasi (MCSOS_BLK_EINVAL)]` |
| `[Buffer cache dengan algoritma clock/second-chance (clock_hand berputar pada array entries)]` | `[LRU murni dengan linked list terurut]` | `[Implementasi lebih sederhana pada memori statis, cukup untuk kebutuhan edukasi milestone ini]` | `[Bukan LRU eksak; entri yang baru diakses dapat tetap tergantikan pada iterasi clock berikutnya]` |
| `[Dirty bit per entri cache dan write-back eksplisit (flush_entry, flush_all)]` | `[Write-through langsung ke device pada setiap write]` | `[Mengurangi jumlah I/O fisik pada beban tulis berulang ke blok yang sama]` | `[Perlu flush eksplisit sebelum shutdown/unmount agar tidak kehilangan data]` |

### 9.3 Arsitektur Ringkas

```text
Kernel (kmain) — M14 bring-up
     │
     ├──► VFS/ramfs (M13) — abstraksi file di memori
     │
     └──► Block Device Layer (M14)
              │
              ├──► mcsos_blk_register() → registry (g_blk_devices[])
              │
              ├──► mcsos_ramblk_init() → driver RAM block device
              │        (mcsos_ramblk_read/write/flush via mcsos_blk_ops_t)
              │
              ├──► mcsos_blk_read()/mcsos_blk_write()/mcsos_blk_flush()
              │        │
              │        ▼
              │   mcsos_blk_validate_range() (cek batas LBA)
              │        │
              │        ▼
              │   dev->ops->read/write/flush(dev, lba, count, buffer)
              │
              └──► Buffer Cache (mcsos_bcache_t)
                       │
                       ├──► mcsos_bcache_read()/write() → cari entri (mcsos_bcache_find)
                       │        │
                       │        ▼
                       │   jika tidak ada: mcsos_bcache_select_victim()
                       │        (clock/second-chance pada cache->entries[])
                       │        │
                       │        ▼
                       │   jika korban dirty: mcsos_bcache_flush_entry()
                       │        (write-back ke device via mcsos_blk_write)
                       │
                       └──► mcsos_bcache_flush_all() → flush seluruh entri dirty
```

Penjelasan diagram:

```text
Kernel memanggil mcsos_blk_register() untuk mendaftarkan device blok
(mis. RAM block device dari mcsos_ramblk_init()) ke dalam registry
statis. Operasi baca/tulis generik (mcsos_blk_read/write) selalu
memvalidasi rentang LBA melalui mcsos_blk_validate_range() sebelum
meneruskan panggilan ke fungsi ops device yang bersangkutan. Buffer
cache berada di atas device blok: saat mcsos_bcache_read()/write()
dipanggil, cache mencari entri yang sudah menampung LBA yang diminta
(mcsos_bcache_find); jika tidak ditemukan, cache memilih korban melalui
algoritma clock/second-chance (mcsos_bcache_select_victim), melakukan
write-back terlebih dahulu jika entri korban berstatus dirty, lalu
memuat data dari device ke entri tersebut. Penulisan melalui cache
hanya menandai entri sebagai dirty tanpa langsung menulis ke device;
write-back sesungguhnya baru terjadi saat entri digantikan atau saat
mcsos_bcache_flush_all() dipanggil secara eksplisit.
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[mcsos_blk_register(dev)]` | `[kernel bring-up / driver init]` | `[registry device blok]` | `[dev, dev->ops, ops->read, ops->write tidak NULL; name tidak kosong; block_count > 0; block_size power-of-two dan >= 512]` | `[device tercatat pada g_blk_devices[] dan g_blk_count bertambah]` | `[MCSOS_BLK_EINVAL jika precondition gagal; MCSOS_BLK_EFULL jika registry penuh]` |
| `[mcsos_blk_read(dev, lba, count, buffer)/mcsos_blk_write(...)]` | `[buffer cache, filesystem, atau kernel subsystem lain]` | `[driver device blok]` | `[dev, buffer tidak NULL; count > 0; lba+count tidak melebihi block_count]` | `[data dibaca/ditulis pada rentang LBA yang diminta]` | `[MCSOS_BLK_EINVAL jika parameter tidak valid; MCSOS_BLK_ERANGE jika di luar batas]` |
| `[mcsos_bcache_read(cache, dev, lba, buffer)/mcsos_bcache_write(...)]` | `[filesystem/kernel]` | `[buffer cache]` | `[cache->block_size == dev->block_size]` | `[buffer berisi salinan blok terbaru dari cache; write menandai entri dirty]` | `[MCSOS_BLK_EINVAL jika parameter tidak valid atau ukuran blok tidak cocok]` |
| `[mcsos_bcache_flush_all(cache)]` | `[kernel saat shutdown/checkpoint]` | `[buffer cache dan device blok]` | `[cache sudah diinisialisasi]` | `[seluruh entri dirty ditulis ke device dan dirty di-clear]` | `[Mengembalikan status error pertama dari mcsos_blk_write jika write-back gagal]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `[mcsos_blk_device_t]` `` | `[name, block_size, block_count, flags, ops, driver_data]` | `[kernel/driver (block.c, ramblk.c)]` | `[Selama device terdaftar di registry]` | `[block_size harus power-of-two dan >= MCSOS_BLK_DEFAULT_SECTOR_SIZE]` |
| `` `[mcsos_ramblk_t]` `` | `[storage, storage_size]` | `[driver RAM block device (ramblk.c)]` | `[Selama device RAM aktif]` | `[storage_size harus kelipatan block_size]` |
| `` `[mcsos_bcache_entry_t]` `` | `[data, capacity, lba, valid, dirty, dev]` | `[buffer cache (bcache.c)]` | `[Selama cache aktif]` | `[Entri dirty wajib di-flush sebelum digantikan]` |
| `` `[mcsos_bcache_t]` `` | `[entries, entry_count, data_pool, block_size, clock_hand]` | `[kernel (bcache.c)]` | `[Selama cache diinisialisasi]` | `[clock_hand harus berada pada rentang [0, entry_count)]` |

### 9.6 Invariants

1. `Setiap operasi mcsos_blk_read/write yang berhasil harus melalui mcsos_blk_validate_range() terlebih dahulu sehingga lba+count tidak pernah melebihi dev->block_count.`
2. `Entri buffer cache yang dirty tidak boleh digantikan (mcsos_bcache_select_victim) tanpa terlebih dahulu dilakukan write-back melalui mcsos_bcache_flush_entry().`
3. `Registry device blok tidak boleh menerima device dengan ops->read atau ops->write NULL (MCSOS_BLK_EINVAL).`
4. `Fungsi mcsos_bcache_read/write hanya boleh dipakai jika cache->block_size sama dengan dev->block_size.`
5. `Tidak terdapat undefined symbol pada objek m14_block_layer.o hasil kompilasi freestanding.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[Registry g_blk_devices[]/g_blk_count]` | `[kernel (block.c)]` | `[none]` | `[belum diverifikasi]` | `[Belum ada mekanisme locking; asumsi single-core pada M14]` |
| `[mcsos_bcache_t dan entries[]]` | `[kernel (bcache.c)]` | `[none]` | `[tidak]` | `[Akses konkuren ke cache belum dilindungi; berpotensi race pada milestone SMP mendatang]` |
| `[RAM block device storage]` | `[driver ramblk.c]` | `[none]` | `[tidak]` | `[Diasumsikan hanya diakses melalui mcsos_blk_read/write yang sudah tervalidasi]` |

Lock order yang berlaku:

```text
Belum menggunakan mekanisme locking karena kernel masih berjalan pada
konfigurasi single-core, konsisten dengan pendekatan milestone
sebelumnya. Pada milestone SMP (M15) di masa depan, registry device
blok dan struktur buffer cache perlu dilindungi dengan spinlock agar
aman terhadap akses konkuren dari beberapa core.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Duplikasi definisi fungsi bcache (mcsos_bcache_init/read/write/flush_all) akibat kesalahan salin-tempel ke ramblk.c]` | `[ramblk.c, ditemukan saat make m14-host-test]` | `[Kode bcache dihapus dari ramblk.c melalui nano, hanya disimpan pada bcache.c]` | `[ld: "multiple definition of mcsos_bcache_init" pada percobaan build pertama, hilang setelah perbaikan]` |
| `[Akses anggota struct yang tidak ada pada header (dev->priv, ram->data, ram->bytes, dev->read, dev->write)]` | `[ramblk.c: mcsos_ramblk_init dan fungsi read/write/flush]` | `[Kode disesuaikan menggunakan anggota struct yang benar: driver_data, storage, ops, mcsos_copy_name()]` | `[Error clang "no member named 'priv'/'data'/'bytes'" pada make all, hilang setelah perbaikan]` |
| `[Ketidakcocokan tipe pointer fungsi pada .flush = mcsos_ramblk_flush]` | `[ramblk.c: inisialisasi mcsos_blk_ops_t]` | `[Signature fungsi flush disesuaikan agar sesuai tipe mcsos_blk_rw_fn (dev, lba, count, buffer)]` | `[Warning/error -Wincompatible-function-pointer-types hilang setelah perbaikan, make all sukses]` |
| `[Undefined symbol pada objek freestanding]` | `[Linker ld.lld -r]` | `[Audit symbol nm -u build/m14/m14_block_layer.o]` | `[build/m14/m14_nm_undefined.txt kosong]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[Kernel/filesystem → Device blok]` | `[lba dan count yang diminta pemanggil]` | `[mcsos_blk_validate_range() memeriksa lba < block_count dan count tidak melebihi sisa blok]` | `[Mengembalikan MCSOS_BLK_ERANGE, tidak meneruskan I/O di luar batas]` |
| `[Kernel → Registry device blok]` | `[dev yang didaftarkan melalui mcsos_blk_register()]` | `[Pemeriksaan ops, name, block_count, block_size sebelum diterima]` | `[Mengembalikan MCSOS_BLK_EINVAL/MCSOS_BLK_EFULL, device tidak tercatat]` |
| `[Build system → Linker]` | `[objek block.o/ramblk.o/bcache.o]` | `[nm -u untuk memastikan tidak ada unresolved symbol pada m14_block_layer.o]` | `[Target m14-audit menghentikan proses (test ! -s ...) jika ditemukan unresolved symbol]` |

---

## 10. Langkah Kerja Implementasi

### Langkah 1 — `Preflight: memeriksa host, versi toolchain, dan status git sebelum mulai M14`

Maksud langkah:

```text
Memastikan lingkungan WSL siap sebelum membuat cabang kerja M14, serta
mencatat versi toolchain sebagai bukti reproducibility.
```

Perintah:

```bash
cd ~/src/mcsos
git branch
mkdir -p artifacts/m14
{ uname -a; lsb_release -a 2>/dev/null || cat /etc/os-release; } | tee artifacts/m14/host_info.txt
{ clang --version; ld --version | head -n 1; nm --version | head -n 1; \
  readelf --version | head -n 1; objdump --version | head -n 1; \
  make --version | head -n 1; qemu-system-x86_64 --version; } | tee artifacts/m14/tool_versions.txt
```

Output ringkas:

```text
Branch aktif sebelumnya: praktikum-m13-vfs-ramfs
Ubuntu clang version 21.1.8 (6ubuntu1)
GNU ld/nm/readelf/objdump (GNU Binutils for Ubuntu) 2.46
GNU Make 4.4.1
QEMU emulator version 10.2.1
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[host_info.txt]` | `[artifacts/m14/]` | `[Bukti informasi host WSL]` |
| `[tool_versions.txt]` | `[artifacts/m14/]` | `[Bukti versi toolchain]` |

Indikator berhasil:

```text
Kedua file artefak berhasil dibuat via tee tanpa error.
```

### Langkah 2 — `Membuat branch baru praktikum-m14-block-device dan struktur direktori kerja`

Maksud langkah:

```text
Mencabangkan pekerjaan M14 dari branch M13 agar histori commit setiap
milestone tetap terpisah dan mudah ditelusuri.
```

Perintah:

```bash
git switch -c praktikum-m14-block-device
mkdir -p include/mcsos kernel/block tests/host scripts artifacts/m14
```

Output ringkas:

```text
Switched to a new branch 'praktikum-m14-block-device'
```

Indikator berhasil:

```text
Branch baru aktif dan direktori include/mcsos, kernel/block,
tests/host, scripts, artifacts/m14 tersedia.
```

### Langkah 3 — `Membuat script preflight (scripts/m14_preflight.sh)`

Maksud langkah:

```text
Membuat script otomatis untuk memeriksa ketersediaan compiler/linker/
binutils/make/qemu, direktori kerja wajib, dokumen panduan milestone,
serta status kebersihan working tree git sebelum mulai coding M14.
```

Perintah:

```bash
cat > scripts/m14_preflight.sh <<'EOF'
#!/usr/bin/env bash
set -euo pipefail
mkdir -p artifacts/m14
LOG="artifacts/m14/preflight.log"
: > "$LOG"

require_file() { ... }
require_cmd() { ... }

require_cmd clang
require_cmd ld
require_cmd nm
require_cmd readelf
require_cmd objdump
require_cmd sha256sum
require_cmd make
require_cmd qemu-system-x86_64

for d in include kernel tests scripts; do ...; done
for f in OS_panduan_M0.md ... OS_panduan_M13.md; do ...; done

git status --short | tee artifacts/m14/git_status_before_m14.txt
if [[ -s artifacts/m14/git_status_before_m14.txt ]]; then
  echo "WARN: working tree tidak bersih; commit atau stash perubahan sebelum final grading" | tee -a "$LOG"
fi
echo "M14_PREFLIGHT_DONE" | tee -a "$LOG"
EOF
chmod +x scripts/m14_preflight.sh
./scripts/m14_preflight.sh
```

Output ringkas:

```text
OK_CMD: clang, ld, nm, readelf, objdump, sha256sum, make, qemu-system-x86_64
OK_DIR: include, kernel, tests, scripts
WARN_DOC_NOT_FOUND_IN_REPO: OS_panduan_M0.md ... OS_panduan_M13.md (14 dokumen)
?? artifacts/
?? scripts/m14_preflight.sh
WARN: working tree tidak bersih; commit atau stash perubahan sebelum final grading
M14_PREFLIGHT_DONE
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m14_preflight.sh]` | `[scripts/]` | `[Script pemeriksaan lingkungan sebelum coding M14]` |
| `[preflight.log]` | `[artifacts/m14/]` | `[Log hasil preflight]` |
| `[git_status_before_m14.txt]` | `[artifacts/m14/]` | `[Bukti status git sebelum perubahan M14 di-commit]` |

Indikator berhasil (dengan catatan):

```text
Script berhasil dijalankan dan mencetak M14_PREFLIGHT_DONE. Seluruh
tool wajib terverifikasi ada. Peringatan WARN_DOC_NOT_FOUND_IN_REPO
muncul untuk 14 dokumen panduan (OS_panduan_M0.md s.d. M13.md) karena
dokumen panduan tersebut memang tidak disimpan di dalam repository
kode, melainkan terpisah; ini bukan kegagalan melainkan catatan
informasi. Percobaan pertama menulis script sempat gagal (file kosong,
0 byte, chmod: No such file) karena heredoc terputus oleh escape
karakter saat penulisan interaktif; file berhasil dibuat ulang pada
percobaan ketiga.
```

### Langkah 4 — `Menulis header include/mcsos/block.h`

Maksud langkah:

```text
Mendefinisikan tipe data device blok generik (mcsos_blk_device_t,
mcsos_blk_ops_t), status error (mcsos_blk_status_t), tipe RAM block
device (mcsos_ramblk_t), serta tipe buffer cache (mcsos_bcache_t,
mcsos_bcache_entry_t) beserta deklarasi seluruh fungsi publik modul
block layer.
```

Perintah:

```bash
cat > include/mcsos/block.h <<'EOF'
#ifndef MCSOS_BLOCK_H
#define MCSOS_BLOCK_H
#include <stddef.h>
#include <stdint.h>

#define MCSOS_BLK_NAME_MAX 16u
#define MCSOS_BLK_MAX_DEVICES 8u
#define MCSOS_BLK_DEFAULT_SECTOR_SIZE 512u

typedef enum mcsos_blk_status {
    MCSOS_BLK_OK = 0, MCSOS_BLK_EINVAL = -1, MCSOS_BLK_ERANGE = -2,
    MCSOS_BLK_EFULL = -3, MCSOS_BLK_EIO = -4, MCSOS_BLK_ENODEV = -5
} mcsos_blk_status_t;

typedef struct mcsos_blk_device {
    char name[MCSOS_BLK_NAME_MAX];
    uint32_t block_size;
    uint64_t block_count;
    uint32_t flags;
    const mcsos_blk_ops_t *ops;
    void *driver_data;
} mcsos_blk_device_t;
/* ... mcsos_ramblk_t, mcsos_bcache_entry_t, mcsos_bcache_t, deklarasi fungsi ... */
#endif
EOF
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[block.h]` | `[include/mcsos/]` | `[Kontrak tipe dan API modul block layer, 3149 byte]` |

Indikator berhasil:

```text
File terbentuk sepenuhnya (ls -l menunjukkan ukuran 3149 byte) tanpa
terpotong oleh escape karakter heredoc.
```

### Langkah 5 — `Menulis kernel/block/block.c (registry dan operasi generik)`

Maksud langkah:

```text
Mengimplementasikan registry device blok (register/get/count),
validasi rentang LBA, serta fungsi baca/tulis/flush generik yang
meneruskan panggilan ke ops device setelah tervalidasi.
```

Perintah:

```bash
cat > kernel/block/block.c <<'EOF'
#include "mcsos/block.h"
static mcsos_blk_device_t *g_blk_devices[MCSOS_BLK_MAX_DEVICES];
static uint32_t g_blk_count;
/* mcsos_blk_registry_reset, mcsos_blk_register, mcsos_blk_get,
   mcsos_blk_count, mcsos_blk_validate_range, mcsos_blk_read/write/flush */
EOF
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[block.c]` | `[kernel/block/]` | `[Implementasi registry dan operasi generik device blok, 3080 byte]` |

Indikator berhasil:

```text
File terbentuk sepenuhnya (3080 byte) sesuai deklarasi pada block.h.
```

### Langkah 6 — `Menulis kernel/block/ramblk.c dan kernel/block/bcache.c`

Maksud langkah:

```text
Mengimplementasikan driver RAM block device dan buffer cache dengan
algoritma clock/second-chance pada dua file terpisah.
```

Perintah:

```bash
cat > kernel/block/ramblk.c <<'EOF'
#include "mcsos/block.h"
/* mcsos_ramblk_init, mcsos_ramblk_read/write/flush,
   serta (secara tidak sengaja) turut memuat implementasi bcache */
EOF

cat > kernel/block/bcache.c <<'EOF'
#include "mcsos/block.h"
/* mcsos_bcache_init/read/write/flush_all dengan clock/second-chance */
EOF
```

Indikator berhasil (sementara):

```text
Kedua file berhasil dibuat (ramblk.c 4751 byte, bcache.c 4751 byte),
namun belakangan diketahui isi ramblk.c juga memuat fungsi bcache
secara duplikat (lihat Langkah 8).
```

### Langkah 7 — `Menulis unit test host (tests/host/test_m14_block.c)`

Maksud langkah:

```text
Menulis unit test host untuk memverifikasi registry, RAM block device,
dan buffer cache tanpa memerlukan hardware nyata maupun QEMU.
```

Perintah:

```bash
cat > tests/m14_block_test.c <<'EOF'
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mcsos/block.h"
/* EXPECT_OK/EXPECT_EQ/EXPECT_STATUS, main(): registrasi RAM block
   device, uji baca/tulis normal dan di luar batas (ERANGE/EINVAL),
   uji buffer cache read/write, flush_all, dan dirty write-back */
EOF
mkdir -p tests/host
mv tests/m14_block_test.c tests/host/test_m14_block.c
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[test_m14_block.c]` | `[tests/host/]` | `[Unit test host modul block layer, 2824 byte]` |

Indikator berhasil:

```text
File berhasil dipindahkan ke tests/host/test_m14_block.c sesuai
konvensi penamaan target Makefile m14-host-test.
```

### Langkah 8 — `Build pertama (make m14-host-test) gagal: multiple definition mcsos_bcache_*`

Maksud langkah:

```text
Menjalankan build host test pertama kali untuk memverifikasi seluruh
sumber block layer dapat dikompilasi dan ditautkan.
```

Perintah:

```bash
make m14-host-test
```

Output ringkas:

```text
/usr/bin/x86_64-linux-gnu-ld.bfd: /tmp/bcache-9333fe.o: in function
`mcsos_bcache_init': bcache.c:(.text+0x0): multiple definition of
`mcsos_bcache_init'; ramblk.c:(.text+0x0): first defined here
(berulang untuk mcsos_bcache_read, mcsos_bcache_write, mcsos_bcache_flush_all)
undefined reference to `mcsos_ramblk_init'
```

Indikator berhasil (temuan masalah):

```text
Ditemukan bahwa ramblk.c ternyata juga memuat definisi fungsi
mcsos_bcache_init/read/write/flush_all yang seharusnya hanya berada
di bcache.c (grep -n "mcsos_bcache" kernel/block/ramblk.c menunjukkan
fungsi-fungsi tersebut ada di ramblk.c), sementara mcsos_ramblk_init
sendiri belum ditemukan (grep -n "mcsos_ramblk" kernel/block/ramblk.c
tidak menghasilkan apa pun pada saat itu).
```

### Langkah 9 — `Memperbaiki ramblk.c: menghapus duplikasi fungsi bcache, menambahkan mcsos_ramblk_init`

Maksud langkah:

```text
Membersihkan ramblk.c agar hanya memuat implementasi driver RAM block
device, dan memastikan mcsos_ramblk_init benar-benar terdefinisi.
```

Perintah:

```bash
nano kernel/block/ramblk.c
grep -n "mcsos_ramblk_init" kernel/block/ramblk.c
make m14-host-test
```

Output ringkas:

```text
kernel/block/ramblk.c:62:mcsos_blk_status_t mcsos_ramblk_init( ... )

Build masih gagal dengan error baru:
error: no member named 'priv' in 'struct mcsos_blk_device'
error: no member named 'data' in 'struct mcsos_ramblk'
error: no member named 'bytes' in 'struct mcsos_ramblk'
error: array type 'char[16]' is not assignable (dev->name = name;)
error: no member named 'read'/'write' in 'struct mcsos_blk_device'
10 errors generated.
```

Indikator berhasil (temuan masalah lanjutan):

```text
Setelah duplikasi fungsi bcache dibersihkan, ditemukan bahwa isi fungsi
mcsos_ramblk_init dan mcsos_ramblk_read/write/flush pada ramblk.c masih
merujuk ke anggota struct lama yang tidak sesuai dengan block.h
(dev->priv, ram->data, ram->bytes, dev->name = name, dev->read,
dev->write), padahal block.h yang benar menggunakan driver_data,
storage, mcsos_copy_name(), dan ops (mcsos_blk_ops_t) berbasis fungsi
pointer read/write/flush.
```

### Langkah 10 — `Memperbaiki ramblk.c agar sesuai anggota struct pada block.h`

Maksud langkah:

```text
Menyesuaikan seluruh akses anggota struct pada ramblk.c dengan
definisi resmi block.h: dev->driver_data, ram->storage,
mcsos_copy_name(dev->name, name), dev->ops (bukan dev->read/write
langsung).
```

Perintah:

```bash
nano kernel/block/ramblk.c
make all | tee artifacts/m14/m14_make_all.log
```

Output ringkas (error yang tersisa):

```text
kernel/block/ramblk.c:72:14: error: incompatible function pointer
types initializing 'mcsos_blk_rw_fn' ... with an expression of type
'mcsos_blk_status_t (mcsos_blk_device_t *)' [-Wincompatible-function-pointer-types]
   .flush = mcsos_ramblk_flush
1 error generated.
```

Indikator berhasil (temuan masalah lanjutan):

```text
Error sebelumnya (priv/data/bytes/name/read/write) sudah hilang;
tersisa satu error terkait signature fungsi mcsos_ramblk_flush yang
belum menerima parameter (dev, lba, count, buffer) sesuai tipe
mcsos_blk_rw_fn yang dipakai bersama untuk read/write/flush.
```

### Langkah 11 — `Memperbaiki signature mcsos_ramblk_flush dan menjalankan make all secara penuh`

Maksud langkah:

```text
Menyesuaikan signature mcsos_ramblk_flush agar kompatibel dengan tipe
mcsos_blk_rw_fn, lalu menjalankan build freestanding penuh (make all)
untuk memastikan seluruh modul kernel (termasuk block layer) berhasil
dikompilasi dan ditautkan menjadi kernel.elf.
```

Perintah:

```bash
nano kernel/block/ramblk.c
make all | tee artifacts/m14/m14_make_all.log
```

Output ringkas:

```text
clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding ... \
  -c kernel/block/ramblk.c -o build/normal/kernel/block/ramblk.o
(berhasil, tanpa error)
ld.lld -nostdlib -static -z max-page-size=0x1000 -T linker.ld \
  -Map=build/kernel.map -o build/kernel.elf ... build/normal/kernel/block/*.o ...
readelf -h build/kernel.elf > build/kernel.readelf.header.txt
nm -n build/kernel.elf > build/kernel.syms.txt
objdump -d -Mintel build/kernel.elf > build/kernel.disasm.txt
grep -q 'ELF64' build/kernel.readelf.header.txt
grep -q 'Machine: ... X86-64' build/kernel.readelf.header.txt
grep -q 'kmain' build/kernel.syms.txt
grep -q 'x86_64_idt_init' build/kernel.syms.txt
grep -q 'x86_64_trap_dispatch' build/kernel.syms.txt
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[kernel.elf, kernel.map]` | `[build/]` | `[Kernel freestanding yang telah memuat modul block layer M14]` |
| `[m14_make_all.log]` | `[artifacts/m14/]` | `[Log lengkap build make all]` |

Indikator berhasil:

```text
make all selesai tanpa error; seluruh grep verifikasi ELF64, arsitektur
X86-64, simbol kmain/x86_64_idt_init/x86_64_trap_dispatch, serta
instruksi iretq/lidt pada disassembly berhasil (tidak ada exit code
non-zero yang dilaporkan).
```

### Langkah 12 — `Menjalankan target m14-all (host test + audit freestanding)`

Maksud langkah:

```text
Menjalankan unit test host block layer sekaligus membangun objek
freestanding gabungan (m14_block_layer.o) dan mengaudit unresolved
symbol serta struktur ELF-nya.
```

Perintah:

```bash
make m14-all
```

Output ringkas:

```text
clang -std=c17 -Wall -Wextra -Werror -Iinclude \
  tests/host/test_m14_block.c kernel/block/block.c kernel/block/ramblk.c kernel/block/bcache.c \
  -o build/m14/test_m14_block
./build/m14/test_m14_block
M14 host tests PASS

clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding -fno-builtin \
  -fno-stack-protector -fno-pic -mno-red-zone -Wall -Wextra -Werror -Iinclude \
  -c kernel/block/block.c -o build/m14/block.o
(idem untuk ramblk.c dan bcache.c)

ld.lld -r -o build/m14/m14_block_layer.o build/m14/block.o build/m14/ramblk.o build/m14/bcache.o
nm -u build/m14/m14_block_layer.o > build/m14/m14_nm_undefined.txt
test ! -s build/m14/m14_nm_undefined.txt
readelf -h build/m14/m14_block_layer.o > build/m14/m14_readelf.txt
objdump -dr build/m14/m14_block_layer.o > build/m14/m14_objdump.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[test_m14_block]` | `[build/m14/]` | `[Binary unit test host, 21624 byte]` |
| `[block.o, ramblk.o, bcache.o]` | `[build/m14/]` | `[Objek freestanding masing-masing modul]` |
| `[m14_block_layer.o]` | `[build/m14/]` | `[Objek gabungan hasil ld.lld -r, 6456 byte]` |
| `[m14_nm_undefined.txt]` | `[build/m14/]` | `[Bukti tidak ada unresolved symbol, 0 byte]` |
| `[m14_readelf.txt, m14_objdump.txt]` | `[build/m14/]` | `[Bukti struktur ELF dan disassembly objek block layer]` |

Indikator berhasil:

```text
Unit test host lulus dengan pesan "M14 host tests PASS". Build
freestanding block.o/ramblk.o/bcache.o sukses tanpa error. File
m14_nm_undefined.txt berukuran 0 byte sehingga perintah
"test ! -s ..." lulus, membuktikan tidak ada unresolved symbol pada
m14_block_layer.o.
```

### Langkah 13 — `Commit dan push hasil pekerjaan M14 ke branch praktikum-m14-block-device`

Maksud langkah:

```text
Menyimpan seluruh perubahan (header, sumber block layer, unit test,
script preflight, artefak) ke git dan mendorongnya ke remote GitHub
sebagai bukti penyelesaian praktikum M14.
```

Perintah:

```bash
git status
git add Makefile include/mcsos/block.h kernel/block scripts/m14_preflight.sh tests/host artifacts
git status
git commit -m "praktikum M14: add block device layer and buffer cache"
git log --oneline -1
git push -u origin praktikum-m14-block-device
```

Output ringkas:

```text
[praktikum-m14-block-device a64ca8e] praktikum M14: add block device layer and buffer cache
 10 files changed, 634 insertions(+)
a64ca8e (HEAD -> praktikum-m14-block-device) praktikum M14: add block device layer and buffer cache

Enumerating objects: 28, done.
Writing objects: 100% (21/21), 6.44 KiB | 439.00 KiB/s, done.
remote: Create a pull request for 'praktikum-m14-block-device' on GitHub by visiting:
remote:      https://github.com/syifanurzimah/MCSOS/pull/new/praktikum-m14-block-device
 * [new branch]      praktikum-m14-block-device -> praktikum-m14-block-device
branch 'praktikum-m14-block-device' set up to track 'origin/praktikum-m14-block-device'.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[commit a64ca8e]` | `[branch praktikum-m14-block-device]` | `[Snapshot final pekerjaan M14 (10 file, 634 baris)]` |

Indikator berhasil:

```text
Commit berhasil dibuat dan branch praktikum-m14-block-device berhasil
di-push ke origin sebagai branch baru pada remote GitHub.
```

---

## 11. Checkpoint Buildable

| Checkpoint | Perintah | Status |
|---|---|---|
| `[Host test build]` | `[make m14-host-test]` | `[PASS — "M14 host tests PASS"]` |
| `[Freestanding per-file build]` | `[make m14-freestanding]` | `[PASS — block.o/ramblk.o/bcache.o terbentuk]` |
| `[Audit unresolved symbol]` | `[make m14-audit]` | `[PASS — m14_nm_undefined.txt kosong]` |
| `[Build kernel penuh]` | `[make all]` | `[PASS — kernel.elf terbentuk, seluruh grep verifikasi lulus]` |
| `[Target gabungan]` | `[make m14-all]` | `[PASS — mencakup m14-host-test dan m14-audit]` |

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

```bash
make clean || true
make all | tee artifacts/m14/m14_make_all.log
```

### 12.2 Static Inspection

```bash
nm -u build/m14/m14_block_layer.o > build/m14/m14_nm_undefined.txt
test ! -s build/m14/m14_nm_undefined.txt
readelf -h build/m14/m14_block_layer.o
objdump -dr build/m14/m14_block_layer.o
```

### 12.3 Unit Test (Host)

```bash
make m14-host-test
./build/m14/test_m14_block
```

Output:

```text
M14 host tests PASS
```

### 12.4 Static Grading Menyeluruh

```bash
make m14-all
```

Output ringkas:

```text
M14 host tests PASS
(build block.o/ramblk.o/bcache.o freestanding sukses)
(ld.lld -r berhasil, nm -u kosong, readelf dan objdump berhasil ditulis)
```

### 12.5 GDB Debug Evidence

```text
Belum dilakukan pada sesi M14 ini. Direncanakan pada iterasi
berikutnya untuk memverifikasi runtime mcsos_blk_read/write dan
mcsos_bcache_read/write melalui breakpoint GDB pada QEMU.
```

### 12.6 Stress/Fuzz/Fault Injection Test

```text
Unit test host (test_m14_block.c) mencakup kasus batas: baca/tulis di
luar rentang LBA (MCSOS_BLK_ERANGE), count=0 (MCSOS_BLK_EINVAL), dan
buffer NULL (MCSOS_BLK_EINVAL), namun belum ada stress test beban
tinggi maupun fuzzing acak pada sesi ini.
```

### 12.7 Visual Evidence

```text
Belum ada screenshot yang dilampirkan secara terpisah pada sesi ini;
seluruh bukti berupa output terminal yang tercatat pada transkrip dan
file log artifacts/m14/m14_make_all.log.
```

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| Pengujian | Hasil | Bukti |
|---|---|---|
| `[Unit test host block layer]` | `[PASS]` | `["M14 host tests PASS" pada output ./build/m14/test_m14_block]` |
| `[Build freestanding penuh (make all)]` | `[PASS]` | `[kernel.elf terbentuk, seluruh grep verifikasi ELF64/simbol/instruksi lulus]` |
| `[Audit unresolved symbol (nm -u)]` | `[PASS]` | `[build/m14/m14_nm_undefined.txt berukuran 0 byte]` |
| `[Preflight lingkungan]` | `[PASS dengan catatan]` | `[Seluruh tool wajib OK; 14 dokumen panduan tidak ditemukan di repo — bukan kegagalan build]` |

### 13.2 Log Penting

```text
M14 host tests PASS

ld.lld -r -o build/m14/m14_block_layer.o build/m14/block.o build/m14/ramblk.o build/m14/bcache.o
nm -u build/m14/m14_block_layer.o > build/m14/m14_nm_undefined.txt
test ! -s build/m14/m14_nm_undefined.txt
```

### 13.3 Artefak Bukti

| Artefak | Lokasi |
|---|---|
| `[m14_make_all.log]` | `[artifacts/m14/]` |
| `[preflight.log]` | `[artifacts/m14/]` |
| `[m14_nm_undefined.txt (kosong)]` | `[build/m14/]` |
| `[m14_readelf.txt, m14_objdump.txt]` | `[build/m14/]` |
| `[test_m14_block (binary host test)]` | `[build/m14/]` |

---

## 14. Analisis Teknis

### 14.1 Analisis Keberhasilan

```text
Modul block device layer M14 berhasil diimplementasikan secara
lengkap: registry device blok generik, driver RAM block device, dan
buffer cache dengan kebijakan clock/second-chance. Unit test host
lulus seluruhnya, termasuk kasus batas rentang LBA dan validasi
parameter. Build freestanding x86_64 juga berhasil setelah tiga
putaran perbaikan (duplikasi fungsi bcache, anggota struct yang salah,
signature fungsi flush), dan audit nm -u membuktikan tidak ada
unresolved symbol pada objek gabungan m14_block_layer.o.
```

### 14.2 Analisis Kegagalan atau Perbedaan Hasil

```text
Tiga kegagalan build ditemukan dan diperbaiki secara berurutan:
1) Multiple definition mcsos_bcache_* akibat isi ramblk.c ikut memuat
   salinan fungsi bcache yang seharusnya hanya ada di bcache.c.
2) Penggunaan anggota struct yang tidak sesuai header (dev->priv,
   ram->data, ram->bytes, dev->name sebagai lvalue, dev->read/write
   langsung) pada draf awal ramblk.c, sementara block.h yang benar
   menggunakan driver_data, storage, dan ops (mcsos_blk_ops_t).
3) Ketidakcocokan tipe pointer fungsi pada inisialisasi
   .flush = mcsos_ramblk_flush karena signature fungsi belum sesuai
   mcsos_blk_rw_fn.

Selain itu, pengujian runtime melalui QEMU/GDB untuk build M14 secara
spesifik belum dilakukan pada sesi ini; verifikasi baru mencakup host
test dan audit statis freestanding.
```

### 14.3 Perbandingan dengan Teori

```text
Implementasi validasi rentang LBA sebelum I/O dan pemisahan device
blok generik dari driver konkret (RAM block device) konsisten dengan
konsep device driver layering pada teori sistem operasi. Kebijakan
penggantian buffer cache clock/second-chance yang diimplementasikan
merupakan pendekatan praktis dari algoritma second-chance/clock yang
umum dipakai sebagai aproksimasi LRU dengan biaya implementasi lebih
rendah.
```

### 14.4 Kompleksitas dan Kinerja

```text
Registry device blok menggunakan pencarian linear terbatas maksimum 8
device (O(1) praktis). Pencarian entri buffer cache (mcsos_bcache_find)
bersifat linear terhadap jumlah entri cache (O(n) dengan n = entry_count),
demikian pula pemilihan korban (mcsos_bcache_select_victim) yang
melakukan iterasi clock hingga satu putaran penuh entri cache dalam
kasus terburuk. Untuk ukuran cache kecil pada pengujian (2 entri),
biaya ini dapat diabaikan.
```

---

## 15. Debugging dan Failure Modes

### 15.1 Failure Modes yang Ditemukan

| Failure mode | Gejala | Akar masalah | Perbaikan |
|---|---|---|---|
| `[Multiple definition fungsi bcache]` | `[Linker error saat make m14-host-test]` | `[Kode bcache tersalin ganda ke ramblk.c]` | `[Menghapus salinan fungsi bcache dari ramblk.c]` |
| `[Anggota struct tidak ditemukan]` | `[Error compile: no member named 'priv'/'data'/'bytes'/'read'/'write']` | `[ramblk.c ditulis mengacu struct lama yang tidak sesuai block.h final]` | `[Menyesuaikan ke driver_data, storage, ops, mcsos_copy_name()]` |
| `[Incompatible function pointer types pada .flush]` | `[Error compile saat make all]` | `[Signature mcsos_ramblk_flush tidak sesuai mcsos_blk_rw_fn]` | `[Menyesuaikan signature fungsi flush]` |
| `[Script preflight kosong/gagal chmod]` | `[chmod: cannot access ... No such file or directory]` | `[Heredoc terputus oleh escape karakter saat pengetikan interaktif]` | `[Menulis ulang heredoc hingga file terbentuk penuh]` |

### 15.2 Failure Modes yang Diantisipasi

```text
Buffer cache belum diuji pada skenario seluruh entri dirty dan device
gagal menulis (I/O error) sehingga mcsos_bcache_flush_entry akan
mengembalikan error dari mcsos_blk_write; perilaku kernel terhadap
error tersebut pada level pemanggil di luar unit test belum dirancang
secara eksplisit pada M14.
```

### 15.3 Triage yang Dilakukan

```text
Setiap error compile/link ditelusuri satu per satu: pesan linker
"multiple definition" diperiksa melalui grep -n "mcsos_bcache"
kernel/block/ramblk.c untuk memastikan lokasi duplikasi; pesan compile
"no member named" ditelusuri ke definisi struct pada block.h
(sed -n '1,220p' include/mcsos/block.h) untuk memastikan nama anggota
yang benar; pesan "incompatible function pointer types" ditelusuri ke
tipe mcsos_blk_rw_fn pada header. Setiap perbaikan diverifikasi ulang
dengan menjalankan kembali make m14-host-test atau make all.
```

### 15.4 Panic Path

```text
Modul block layer M14 tidak menambahkan jalur panic baru; kegagalan
operasi dikembalikan sebagai kode status mcsos_blk_status_t
(MCSOS_BLK_EINVAL/ERANGE/EFULL/EIO/ENODEV) kepada pemanggil, bukan
memicu panic kernel secara langsung.
```

---

## 16. Prosedur Rollback

```text
Karena seluruh pekerjaan M14 berada pada branch terpisah
(praktikum-m14-block-device) yang dicabangkan dari praktikum-m13-vfs-ramfs,
rollback dapat dilakukan dengan git switch praktikum-m13-vfs-ramfs
untuk kembali ke kondisi sebelum M14, atau git reset --hard terhadap
commit sebelum a64ca8e pada branch M14 itu sendiri jika perubahan
perlu dibatalkan tanpa menghapus branch. Belum ada mekanisme rollback
runtime (mis. snapshot device blok) karena RAM block device pada M14
bersifat volatile dan direset setiap kali proses unit test dijalankan
ulang.
```

---

## 17. Keamanan dan Reliability

### 17.1 Risiko Keamanan

| Risiko | Mitigasi saat ini | Mitigasi yang masih diperlukan |
|---|---|---|
| `[Akses I/O di luar batas LBA]` | `[mcsos_blk_validate_range() memeriksa lba dan count]` | `[Belum ada pembatasan hak akses per-caller (mis. capability/ACL) — direncanakan pada M14 versi security roadmap]` |
| `[Registrasi device blok oleh kode tidak tepercaya]` | `[Validasi ops, name, block_count, block_size]` | `[Belum ada verifikasi identitas/izin pemanggil mcsos_blk_register()]` |

### 17.2 Reliability dan Data Integrity

```text
Buffer cache menjamin data yang ditulis melalui mcsos_bcache_write
akhirnya ditulis ke device melalui write-back (baik saat entri
digantikan maupun saat mcsos_bcache_flush_all dipanggil), namun jika
kernel berhenti tanpa memanggil flush_all, data pada entri dirty yang
belum di-flush akan hilang karena RAM block device bersifat volatile.
Unit test host memverifikasi bahwa setelah flush_all, data yang dibaca
langsung dari device (mcsos_blk_read) sama dengan data yang ditulis
melalui cache.
```

### 17.3 Negative Test

```text
Unit test host mencakup negative test: baca pada lba di luar batas
(mcsos_blk_read(&dev, 32u, 1u, out) menghasilkan MCSOS_BLK_ERANGE
karena block_count device hanya 32), tulis dengan count melebihi sisa
blok (MCSOS_BLK_ERANGE), tulis dengan count=0 (MCSOS_BLK_EINVAL), dan
tulis dengan buffer NULL (MCSOS_BLK_EINVAL).
```

---

## 18. Pembagian Kerja Kelompok

```text
Praktikum M14 ini dikerjakan secara individu oleh Syifa Nurzimah
(25832074009), mencakup seluruh tahap: penulisan header/implementasi,
penulisan unit test, perbaikan build, hingga commit dan push ke
repository.
```

### 18.1 Mekanisme Koordinasi

```text
Tidak berlaku (pengerjaan individu).
```

### 18.2 Evaluasi Kontribusi

| Nama | Kontribusi |
|---|---|
| `[Syifa Nurzimah]` | `[100% — seluruh implementasi, debugging, unit test, dan dokumentasi M14]` |

---

## 19. Kriteria Lulus Praktikum

| Kriteria | Terpenuhi? | Bukti |
|---|---|---|
| `[Build freestanding berhasil tanpa error]` | `[Ya]` | `[make all selesai, kernel.elf terbentuk]` |
| `[Unit test host lulus]` | `[Ya]` | `["M14 host tests PASS"]` |
| `[Tidak ada unresolved symbol pada objek block layer]` | `[Ya]` | `[m14_nm_undefined.txt kosong]` |
| `[Perubahan dikomit dan di-push ke repository]` | `[Ya]` | `[commit a64ca8e pada branch praktikum-m14-block-device]` |
| `[Bukti runtime QEMU/GDB untuk build M14]` | `[Belum]` | `[Belum dilakukan pada sesi ini]` |

---

## 20. Readiness Review

```text
Status: Siap uji pada level build dan unit test host/freestanding.
Modul block device layer (registry, RAM block device, buffer cache)
telah lulus unit test host dan audit statis (nm/readelf/objdump) tanpa
unresolved symbol. Namun demikian, verifikasi runtime melalui QEMU
dengan sesi GDB pada fungsi mcsos_blk_read/write dan
mcsos_bcache_read/write belum dilakukan pada sesi ini, sehingga klaim
readiness dibatasi pada level build dan host test, konsisten dengan
prinsip tidak mengklaim status yang melebihi bukti yang tersedia.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Skor yang diklaim | Bukti |
|---|---|---|---|
| `[Implementasi registry & operasi generik device blok]` | `[25]` | `[25]` | `[block.c, unit test registrasi dan I/O tervalidasi]` |
| `[Implementasi RAM block device]` | `[15]` | `[15]` | `[ramblk.c, mcsos_ramblk_init teruji]` |
| `[Implementasi buffer cache clock/second-chance]` | `[25]` | `[25]` | `[bcache.c, unit test read/write/flush_all]` |
| `[Build freestanding & audit statis]` | `[20]` | `[20]` | `[make all sukses, nm -u kosong]` |
| `[Dokumentasi & pengelolaan versi (git)]` | `[15]` | `[13]` | `[commit a64ca8e, laporan ini; belum ada bukti runtime QEMU/GDB]` |
| **Total** | **100** | **`[98]`** | |

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Block device layer M14 berhasil diimplementasikan secara fungsional:
registry device blok, RAM block device, dan buffer cache dengan
kebijakan clock/second-chance seluruhnya lulus unit test host dan
berhasil dikompilasi pada mode freestanding x86_64 tanpa unresolved
symbol.
```

### 22.2 Yang Belum Berhasil

```text
Verifikasi runtime melalui QEMU/GDB terhadap build M14 belum
dilakukan; stress test/fuzzing pada buffer cache dengan beban tinggi
juga belum dilaksanakan pada sesi ini.
```

### 22.3 Rencana Perbaikan

```text
Rencana selanjutnya adalah melakukan boot kernel M14 pada QEMU dan
sesi GDB dengan breakpoint pada mcsos_blk_read, mcsos_blk_write,
mcsos_bcache_read, dan mcsos_bcache_write untuk melengkapi bukti
runtime M14. Selanjutnya dilanjutkan implementasi filesystem persisten
(mcsfs/ext2-like) yang akan memanfaatkan block device layer dan buffer
cache ini sebagai lapisan penyimpanan di bawahnya.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
git log --oneline -1
a64ca8e (HEAD -> praktikum-m14-block-device) praktikum M14: add block device layer and buffer cache
```

### Lampiran B — Diff Ringkas

```diff
git diff --stat (ringkasan commit)
10 files changed, 634 insertions(+)
 Makefile                            |  ...
 artifacts/m14/git_status_before_m14.txt |  ... (baru)
 artifacts/m14/host_info.txt         |  ... (baru)
 artifacts/m14/tool_versions.txt     |  ... (baru)
 include/mcsos/block.h               |  ... (baru)
 kernel/block/bcache.c               |  ... (baru)
 kernel/block/block.c                |  ... (baru)
 kernel/block/ramblk.c               |  ... (baru)
 scripts/m14_preflight.sh            |  ... (baru)
 tests/host/test_m14_block.c         |  ... (baru)
```

### Lampiran C — Log Build Lengkap

```text
Build freestanding (make all) berhasil tanpa error akhir setelah tiga
putaran perbaikan pada ramblk.c. Seluruh objek block.o, ramblk.o, dan
bcache.o berhasil dikompilasi dan dilink ke kernel.elf. Target
tambahan m14-all (host test + audit freestanding) juga berhasil
seluruhnya.
```

### Lampiran D — Log QEMU

```text
Belum tersedia pada sesi M14 ini. Sesi QEMU/GDB untuk build M14 secara
spesifik direncanakan pada iterasi berikutnya (lihat bagian 20 dan
22.3).
```

### Lampiran E — Output Readelf/Objdump

```text
build/m14/m14_readelf.txt
build/m14/m14_objdump.txt
build/m14/m14_nm_undefined.txt (kosong — tidak ada unresolved symbol)
build/kernel.readelf.header.txt
build/kernel.readelf.programs.txt
build/kernel.syms.txt
build/kernel.disasm.txt
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[Screenshot Terminal]` | `[make all dan make m14-all berhasil]` |
| 2 | `[Screenshot unit test host]` | `[M14 host tests PASS]` |
| 3 | `[Screenshot git commit/push]` | `[commit a64ca8e pada branch praktikum-m14-block-device]` |

### Lampiran G — Bukti Tambahan

```text
build/m14/
build/normal/kernel/block/block.o
build/normal/kernel/block/ramblk.o
build/normal/kernel/block/bcache.o
```

---

## 24. Daftar Referensi

```text
[1] R. H. Arpaci-Dusseau and A. C. Arpaci-Dusseau, Operating Systems: Three Easy Pieces. Available: https://pages.cs.wisc.edu/~remzi/OSTEP/
[2] LLVM Project Documentation. Available: https://clang.llvm.org/docs/
[3] GNU Binutils Documentation. Available: https://sourceware.org/binutils/docs/
[4] Git Documentation. Available: https://git-scm.com/doc
```

Referensi yang benar-benar dipakai dalam laporan:

```text
[1] Operating Systems: Three Easy Pieces, bab I/O Devices dan Buffer Cache.
[2] LLVM/Clang Documentation, opsi --target=x86_64-unknown-none-elf, -ffreestanding.
[3] GNU Binutils Documentation, nm -u, readelf -h, objdump -dr.
[4] Dokumentasi Git, git switch -c, git status --short, git add/commit/push.
```

---

## 25. Checklist Final Sebelum Pengumpulan

| Checklist | Status |
|---|---|
| Semua placeholder `[isi ...]` sudah diganti | `[Sebagian — tanggal pengumpulan final dan hash SHA-256 kernel.elf perlu diisi]` |
| Metadata laporan lengkap | `[Ya]` |
| Commit awal dan akhir dicatat | `[Commit akhir a64ca8e dicatat; commit awal hanya berupa referensi branch M13 karena hash tidak tercatat eksplisit pada transkrip]` |
| Perintah build dan test dapat dijalankan ulang | `[Ya]` |
| Log build dilampirkan | `[Ya]` |
| Log unit test host dilampirkan | `[Ya]` |
| Log QEMU/test runtime untuk build M14 dilampirkan | `[Belum]` |
| Artefak penting diberi hash | `[Belum — perlu sha256sum build/kernel.elf]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya — status disesuaikan dengan bukti yang benar-benar tersedia]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
a64ca8e

Status akhir yang diklaim:

Siap uji (build & host test & audit statis); QEMU/GDB untuk build M14
belum tersedia
```

Status akhir yang diklaim:

```text
[Siap uji (build & host test & audit statis)]
```

Ringkasan satu paragraf:

```text
[Praktikum M14 berhasil mengimplementasikan Block Device Layer berupa
registry device blok generik, driver RAM block device, dan buffer
cache dengan kebijakan penggantian clock/second-chance. Tiga masalah
build (duplikasi fungsi bcache pada ramblk.c, ketidaksesuaian anggota
struct, dan ketidakcocokan tipe pointer fungsi flush) berhasil
ditemukan dan diperbaiki secara berurutan hingga unit test host lulus
("M14 host tests PASS") dan build freestanding x86_64 berhasil tanpa
unresolved symbol (nm -u kosong). Bukti runtime QEMU/GDB untuk build
M14 secara spesifik belum tersedia pada sesi ini dan akan dilengkapi
pada iterasi berikutnya. Repository telah diperbarui pada branch
praktikum-m14-block-device (commit a64ca8e) sebagai bukti penyelesaian
praktikum M14 pada level build dan unit test.]
```
