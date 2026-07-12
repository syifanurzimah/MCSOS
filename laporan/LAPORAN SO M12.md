 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M12

**Nama file laporan:** `laporan_praktikum_m12_25832074009.md`
**Nama sistem operasi:** MCSOS versi 260502
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.
**Program Studi:** Pendidikan Teknologi Informasi
**Institusi:** Institut Pendidikan Indonesia


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M12]` |
| Judul praktikum | `[Primitif Sinkronisasi Kernel: Spinlock, Mutex, dan Lock-Dependency Validator (Lockdep) pada MCSOS x86_64]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[isi jika kelompok]` |
| Anggota kelompok | `[tidak berlaku]` |
| Tanggal praktikum | `[2026-07-07]` |
| Tanggal pengumpulan | `[YYYY-MM-DD]` |
| Repository | `[/home/syifa/src/mcsos]` |
| Branch | `[praktikum/m12-sync]` |
| Commit awal | `` `[a701faa — Complete M11 ELF user loader integration]` `` |
| Commit akhir | `` `[673ea92 — Complete M12 synchronization primitives and lockdep]` `` |
| Status readiness yang diklaim | `[Siap uji QEMU]` |

---

## 1. Sampul

# Laporan Praktikum `m12`
## `Primitif Sinkronisasi Kernel: Spinlock, Mutex, dan Lock-Dependency Validator (Lockdep) pada MCSOS x86_64`

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

Saya menyatakan bahwa laporan ini disusun berdasarkan hasil praktikum yang saya kerjakan sendiri menggunakan lingkungan pengembangan WSL 2 Ubuntu sesuai panduan praktikum M12. Seluruh hasil yang dilaporkan didukung oleh bukti berupa command output, log, artefak build, dan commit repository yang terdokumentasi, termasuk proses debugging Makefile.m12 (tiga kelas kegagalan berbeda), pembuatan ulang image ISO, boot QEMU nyata, dan sesi debugging remote GDB yang benar-benar terjadi selama sesi.

| Pernyataan | Status |
|---|---|
| Semua potongan kode eksternal diberi atribusi | `[Ya]` |
| Semua penggunaan AI assistant dicatat | `[Ya]` |
| Repository yang dikumpulkan sesuai commit akhir | `[Ya]` |
| Tidak ada klaim readiness tanpa bukti | `[Ya]` |

Catatan penggunaan bantuan eksternal:

```text
Menggunakan dokumentasi resmi Clang/LLVM, GNU Make, GNU Binutils (nm, readelf, objdump), GNU Assembler/AT&T syntax x86_64, xorriso, Limine bootloader, QEMU, GNU GDB, dan Git sebagai referensi teknis. Menggunakan AI Assistant untuk membantu memahami instruksi praktikum M12, melakukan debugging Makefile.m12 (tiga kelas kegagalan: (1) kernel/sync/selftest.c yang semula ikut dimasukkan ke daftar sumber host-test sehingga header mcsos/kernel/log.h tidak ditemukan pada build host; (2) kesalahan "missing separator" akibat baris resep baru yang tidak konsisten dengan .RECIPEPREFIX/tab; (3) KERNEL_CFLAGS freestanding yang belum memuat -Ikernel/include sehingga header mcsos/kernel/log.h tidak ditemukan saat kompilasi freestanding selftest.c), menjelaskan mekanisme audit nm -u untuk objek yang secara sah masih memiliki simbol undefined sebelum tahap link kernel penuh, serta membantu penyusunan laporan praktikum. Seluruh command, konfigurasi, build, pengujian host, audit objek freestanding, pembuatan ISO, boot QEMU, dan sesi debugging GDB dijalankan serta diperiksa ulang secara mandiri pada lingkungan WSL 2 Ubuntu. Sesi dimulai pada branch praktikum-m11-elf-user-loader (commit akhir a701faa), kemudian dibuka branch baru praktikum/m12-sync untuk pekerjaan M12. Commit akhir repository: 673ea92 ("Complete M12 synchronization primitives and lockdep"), telah dipush ke remote origin/praktikum/m12-sync.
```

---

## 3. Tujuan Praktikum

Tuliskan tujuan teknis dan konseptual praktikum. Tujuan harus dapat diuji.

1. `Mendesain kontrak primitif sinkronisasi MCSOS (mcs_spinlock_t, mcs_mutex_t, mcs_lockdep_state_t) dalam header include/mcs_sync.h sebagai antarmuka tetap untuk seluruh subsistem kernel yang membutuhkan mutual exclusion.`
2. `Mengimplementasikan spinlock berbasis atomic builtin (kernel/sync/spinlock.c: mcs_spin_init, mcs_spin_try_lock, mcs_spin_lock, mcs_spin_unlock, mcs_spin_is_locked) memakai __atomic_exchange_n/__atomic_store_n/__atomic_load_n dan instruksi PAUSE saat busy-wait.`
3. `Mengimplementasikan mutex kooperatif dengan pencatatan owner (kernel/sync/mutex.c: mcs_mutex_init, mcs_mutex_try_lock, mcs_mutex_unlock, mcs_mutex_is_locked, mcs_mutex_owner) sebagai lapisan di atas spinlock/atomic dasar.`
4. `Mengimplementasikan lock-dependency validator (kernel/sync/lockdep.c: mcs_lockdep_init, mcs_lockdep_before_acquire, mcs_lockdep_after_release, mcs_lockdep_is_held) yang mencatat maksimum 16 lock aktif per konteks (MCS_LOCKDEP_MAX_HELD) untuk mendeteksi pelanggaran urutan penguncian.`
5. `Menyusun host unit test (tests/m12_sync_host_test.c) yang menguji spinlock, mutex, dan lockdep sebelum objek freestanding dibangun, dijalankan lewat Makefile.m12 target host-test.`
6. `Mengaudit objek freestanding hasil kompilasi lockdep.c, spinlock.c, dan mutex.c menggunakan nm, readelf, dan objdump untuk membuktikan struktur ELF64 x86_64 dan tidak adanya simbol undefined pada objek inti sinkronisasi.`
7. `Menulis kernel/sync/selftest.c (m12_sync_selftest) yang mengeksekusi spinlock, mutex, dan lockdep secara nyata di dalam kernel, mengintegrasikannya ke kmain() setelah kernel heap (M8) siap dan sebelum scheduler (M9) diinisialisasi.`
8. `Membangun kernel penuh (kernel.elf), membuat ulang image bootable (mcsos.iso) dengan Limine, dan memverifikasi log serial hasil boot QEMU menunjukkan pesan "[M12] sync selftest start" dan "[M12] sync selftest passed".`
9. `Melakukan sesi debugging remote GDB (qemu -s -S, target remote localhost:1234) dengan breakpoint pada m12_sync_selftest, mcs_spin_lock, dan mcs_lockdep_before_acquire untuk memverifikasi jalur eksekusi primitif sinkronisasi di dalam kernel nyata.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Menjelaskan konsep mutual exclusion, busy-waiting spinlock, mutex kooperatif, dan validasi urutan penguncian (lockdep) sebagai pencegah deadlock]` | `[Bagian Dasar Teori Ringkas dan Desain Teknis]` |
| `[Mendesain kontrak mcs_spinlock_t, mcs_mutex_t, dan mcs_lockdep_state_t beserta kode status bergaya errno (MCS_SYNC_OK, MCS_SYNC_EINVAL, MCS_SYNC_EBUSY, MCS_SYNC_EPERM, MCS_SYNC_EDEADLK, MCS_SYNC_EOVERFLOW)]` | `[Isi include/mcs_sync.h]` |
| `[Mengimplementasikan spinlock atomic (xchg + PAUSE) dan mutex ber-owner dalam C17 freestanding]` | `[Isi kernel/sync/spinlock.c, kernel/sync/mutex.c, output make -f Makefile.m12 host-test]` |
| `[Mengimplementasikan lockdep sederhana untuk mendeteksi pelanggaran urutan penguncian]` | `[Isi kernel/sync/lockdep.c]` |
| `[Menyusun host unit test sinkronisasi dan lulus sebelum objek freestanding dibangun]` | `[tests/m12_sync_host_test.c dan output "[PASS] M12 synchronization host tests passed"]` |
| `[Melakukan audit freestanding object dengan nm, readelf, dan objdump untuk lockdep.o, spinlock.o, dan mutex.o]` | `[evidence/M12/nm-undefined.txt, readelf-lockdep.txt, objdump-spinlock.txt]` |
| `[Mengintegrasikan m12_sync_selftest() ke kmain(), membangun image ISO, dan memverifikasi log serial QEMU menunjukkan selftest sinkronisasi berjalan]` | `[Perubahan kernel/core/kmain.c, build/mcsos.iso, evidence/M12/qemu/m12_serial.log]` |
| `[Mendiagnosis dan memperbaiki tiga kelas kegagalan Makefile.m12 (source host-test yang salah, missing separator, include path freestanding yang kurang)]` | `[Bagian 15 Debugging dan Failure Modes]` |
| `[Melakukan sesi debugging remote GDB pada kernel nyata dan menilai kecukupan bukti breakpoint]` | `[Bagian 12.4 dan 15]` |
| `[Menyusun laporan praktikum dengan bukti build, test, audit object, boot QEMU, dan readiness review]` | `[Laporan ini secara keseluruhan]` |

---

## 5. Peta Milestone MCSOS

Centang milestone yang menjadi fokus laporan ini. Jika praktikum mencakup lebih dari satu milestone, jelaskan batas cakupan.

| Milestone | Fokus | Status dalam laporan |
|---|---|---|
| M0 | Requirements, governance, baseline arsitektur | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M1 | Toolchain reproducible, Git, QEMU, GDB, metadata build | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M2 | Boot image, kernel ELF64, early console | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M3 | Panic path, linker map, GDB, observability awal | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M4 | Trap, exception, interrupt, timer | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M5 | PMM, VMM, page table, kernel heap | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M6 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M7 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M8 | Kernel heap allocator (kmem): free-list, split, coalesce, statistik heap | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M9 | Kernel thread dan cooperative scheduler | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M10 | Syscall ABI, dispatcher, dan antarmuka INT 0x80 | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M11 | ELF user loader dan integrasi loader mode pengguna | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M12 | Primitif sinkronisasi kernel: spinlock, mutex, lockdep | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M13 | SMP, scalability, lock stress, NUMA-aware preparation | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M14 | Framebuffer, graphics console, visual regression | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M15 | Virtualization/container subset | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Batas cakupan praktikum:

```text
Praktikum M12 berfokus pada perancangan dan implementasi primitif sinkronisasi kernel (spinlock, mutex, dan lockdep) sebagai lapisan mutual exclusion di atas kernel heap (M8), scheduler kooperatif (M9), syscall ABI (M10), dan ELF user loader (M11) yang telah tersedia dari sesi-sesi sebelumnya. Aktivitas mencakup penyusunan header kontrak sinkronisasi, implementasi spinlock berbasis atomic builtin dan mutex ber-owner, implementasi lockdep sederhana, host unit test, audit objek freestanding, penulisan selftest kernel nyata, integrasi ke kmain(), pembuatan ulang image ISO dengan Limine, verifikasi log serial hasil boot QEMU, dan sesi debugging remote GDB. Penamaan resmi milestone M6 pada peta roadmap umum ("Thread, scheduler, synchronization") tumpang tindih secara konsep dengan penugasan M12 aktual yang diberikan pada semester berjalan (primitif sinkronisasi kernel: spinlock, mutex, lockdep); laporan ini mengikuti panduan M12 aktual yang diberikan (dibuktikan lewat direktori kerja evidence/M12/ dan branch praktikum/m12-sync), bukan penomoran generik pada tabel roadmap — pola yang sama seperti disiplin penamaan M7 vs M10 pada laporan sebelumnya. Praktikum ini belum mencakup locking pada level SMP/multi-CPU sesungguhnya, stress test konkurensi nyata (kernel MCSOS masih single-core/kooperatif), maupun penanganan deadlock otomatis (lockdep pada sesi ini baru mencatat dan mendeteksi pelanggaran urutan, belum mengambil tindakan pemulihan).
```

---

## 6. Dasar Teori Ringkas

Tuliskan teori yang langsung diperlukan untuk memahami praktikum. Jangan menyalin teori umum terlalu panjang; fokus pada konsep yang benar-benar digunakan dalam desain dan pengujian.

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Praktikum M12 berfokus pada mekanisme sinkronisasi kernel, yaitu primitif yang mencegah dua konteks eksekusi (thread, interrupt handler) mengakses data bersama secara bersamaan sehingga menimbulkan race condition. Konsep yang diuji meliputi spinlock (busy-waiting lock berbasis operasi atomic seperti compare-and-swap/exchange, cocok untuk critical section pendek dan konteks yang tidak boleh tidur), mutex (lock yang mencatat pemilik/owner, cocok untuk critical section yang lebih panjang), dan lock-dependency validator/lockdep (mekanisme pembukuan lock yang sedang dipegang oleh suatu konteks untuk mendeteksi potensi pelanggaran urutan penguncian sebelum benar-benar menyebabkan deadlock pada runtime).
```

### 6.2 Konsep Arsitektur x86_64 yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[ELF64 x86_64 relocatable object]` | `[Format objek lockdep.o, spinlock.o, dan mutex.o hasil kompilasi freestanding sebelum ditautkan ke build/kernel.elf]` | `[Output readelf -h build/m12/lockdep.o dan build/kernel.readelf.header.txt]` |
| `[Operasi atomic x86_64: XCHG, PAUSE]` | `[Dasar implementasi mcs_spin_try_lock (xchg) dan busy-wait di mcs_spin_lock (pause) memakai __atomic_exchange_n/__atomic_load_n/__atomic_store_n dengan memory order ACQUIRE/RELEASE/RELAXED]` | `[Disassembly objdump -d build/m12/spinlock.o menunjukkan instruksi xchg %eax,(%rdi) dan pause]` |
| `[Freestanding Environment]` | `[lockdep.c, spinlock.c, mutex.c, dan selftest.c tidak boleh bergantung pada malloc/free/printf libc host]` | `[Flag -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -mno-red-zone pada target freestanding M12]` |
| `[Header lintas-lapisan mcsos/kernel/log.h dan mcsos/kernel/panic.h]` | `[Dipakai oleh kernel/sync/selftest.c untuk mencetak status selftest dan memicu panic bila validasi sinkronisasi gagal, sehingga selftest.c wajib memakai include path kernel (-Ikernel/include), berbeda dari lockdep.c/spinlock.c/mutex.c yang berdiri sendiri]` | `[Kegagalan build sebelum -Ikernel/include ditambahkan pada Langkah 14, lihat Bagian 15]` |
| `[ABI x86_64 System V]` | `[Dasar pemanggilan fungsi antar-objek mcs_spin_lock, mcs_mutex_try_lock, mcs_lockdep_before_acquire dari kernel/sync/selftest.c dan dari kmain()]` | `[Output nm build/kernel.elf | grep mcs_spin dan objdump -d -Mintel build/kernel.disasm.txt]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding untuk kernel/sync/lockdep.c, spinlock.c, mutex.c, selftest.c]` |
| Runtime | `[tanpa hosted libc pada jalur freestanding; host unit test tests/m12_sync_host_test.c memakai libc host (pthread untuk build host) untuk assert/printf]` |
| ABI | `[x86_64 System V, freestanding calling convention]` |
| Compiler flags kritis | `[-ffreestanding, -fno-builtin, -fno-stack-protector, -fno-pic, -mno-red-zone, --target=x86_64-elf (Makefile.m12) / --target=x86_64-unknown-none-elf (Makefile utama), -Iinclude, -Ikernel/include]` |
| Risiko undefined behavior | `[Busy-wait tanpa batas waktu pada mcs_spin_lock bila lock tidak pernah dilepas, potensi priority inversion pada mutex tanpa priority inheritance, overflow pencatatan lockdep bila depth melebihi MCS_LOCKDEP_MAX_HELD (16), dan race pada g_ops/struktur global bila dipanggil dari lebih dari satu konteks tanpa proteksi tambahan]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[Konsep locking (OSTEP): spinlock dan mutex]` | `[Perbandingan busy-waiting vs. blocking lock]` | `[Dasar desain mcs_spin_lock vs. mcs_mutex_try_lock]` |
| `[2]` | `[Intel/AMD manual: instruksi XCHG dan PAUSE]` | `[Semantik atomic exchange dan hint spin-wait]` | `[Dasar implementasi mcs_spin_try_lock dan mcs_cpu_relax]` |
| `[3]` | `[Dokumentasi GNU Binutils]` | `[nm, readelf, dan objdump]` | `[Digunakan untuk memverifikasi objek freestanding lockdep/spinlock/mutex]` |
| `[4]` | `[Materi prasyarat M12: primitif sinkronisasi kernel dan validasi urutan penguncian (lockdep)]` | `[Seluruh bagian prasyarat teori M12]` | `[Menjadi dasar penetapan invariant, ownership, dan security boundary sinkronisasi]` |
| `[5]` | `[Dokumentasi GNU GDB remote debugging (target remote, qemu -s -S)]` | `[Breakpoint pada simbol kernel dan pemeriksaan register/backtrace]` | `[Digunakan pada sesi debugging Bagian 12.4]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows 11 x64]` |
| Lingkungan build | `[WSL 2 Ubuntu, hostname WIN-E2QNIIEGDH4]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-elf (Makefile.m12) / x86_64-unknown-none-elf (Makefile utama)]` |
| Emulator | `[QEMU — berhasil dijalankan pada sesi M12 ini dengan image build/mcsos.iso]` |
| Bootloader/ISO | `[Limine (third_party/limine), xorriso untuk membangun build/mcsos.iso]` |
| Debugger | `[GNU GDB — dipakai pada sesi M12 ini lewat remote debugging qemu -s -S]` |
| Build system | `[GNU Make 4.4.1, Makefile utama dan Makefile.m12 terpisah]` |
| Bahasa utama | `[C17 Freestanding]` |

### 7.2 Versi Toolchain

Tempel output versi toolchain berikut (dari `evidence/M12/preflight.log`).

```bash
date -Is
uname -a
clang --version | head -n 1
cc --version | head -n 1
make --version | head -n 1
git rev-parse --short HEAD
git status --short
```

Output:

```text
2026-07-07T20:30:30+07:00
Linux WIN-E2QNIIEGDH4 6.18.33.2-microsoft-standard-WSL2 #1 SMP PREEMPT_DYNAMIC Thu Jun 18 21:54:43 UTC 2026 x86_64 GNU/Linux
Ubuntu clang version 21.1.8 (6ubuntu1)
cc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
GNU Make 4.4.1
a701faa
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[/home/syifa/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[praktikum/m12-sync]` |
| Commit hash awal (baseline sebelum branch M12 dibuka) | `` `[a701faa — Complete M11 ELF user loader integration]` `` |
| Commit hash akhir | `` `[673ea92]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

```text
mcsos/
├── include/
│   ├── mcs_sync.h
│   └── mcs_sync_selftest.h
├── kernel/
│   ├── core/
│   │   └── kmain.c
│   ├── sync/
│   │   ├── lockdep.c
│   │   ├── spinlock.c
│   │   ├── mutex.c
│   │   └── selftest.c
│   ├── include/mcsos/kernel/log.h
│   └── include/mcsos/kernel/panic.h
├── tests/
│   └── m12_sync_host_test.c
├── build/
│   ├── m12/
│   ├── kernel.elf
│   └── mcsos.iso
├── evidence/
│   └── M12/
│       ├── preflight.log
│       ├── m12-build.log
│       ├── m12-build-after-selftest.log
│       ├── m12-build-final.log
│       ├── nm-undefined.txt
│       ├── readelf-lockdep.txt
│       ├── objdump-spinlock.txt
│       ├── sha256sums.txt
│       ├── m12-sha256-final.txt
│       ├── m12-qemu-serial-final.log
│       ├── gdb-session.log
│       └── qemu/
├── Makefile
├── Makefile.m12
└── tools/scripts/make_iso.sh, scripts/m11_qemu_smoke.sh
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[include/mcs_sync.h]` | `[baru]` | `[Deklarasi kontrak sinkronisasi: mcs_lockdep_state_t, mcs_spinlock_t, mcs_mutex_t, kode status MCS_SYNC_*, dan seluruh deklarasi fungsi lockdep/spinlock/mutex]` | `[tinggi — kontrak ini dipakai bersama oleh lockdep.c, spinlock.c, mutex.c, selftest.c, dan host test]` |
| `[kernel/sync/lockdep.c]` | `[baru]` | `[Implementasi mcs_lockdep_init, mcs_lockdep_before_acquire, mcs_lockdep_after_release, mcs_lockdep_is_held untuk mencatat dan memvalidasi urutan lock yang dipegang]` | `[tinggi]` |
| `[kernel/sync/spinlock.c]` | `[baru]` | `[Implementasi mcs_spin_init, mcs_spin_try_lock, mcs_spin_lock, mcs_spin_unlock, mcs_spin_is_locked memakai __atomic_exchange_n/__atomic_store_n/__atomic_load_n dan instruksi pause]` | `[tinggi — kesalahan memory order dapat menimbulkan race condition]` |
| `[kernel/sync/mutex.c]` | `[baru]` | `[Implementasi mcs_mutex_init, mcs_mutex_try_lock, mcs_mutex_unlock, mcs_mutex_is_locked, mcs_mutex_owner dengan pencatatan owner_id]` | `[tinggi]` |
| `[kernel/sync/selftest.c]` | `[baru]` | `[Fungsi m12_sync_selftest() yang mengeksekusi spinlock, mutex, dan lockdep secara nyata di dalam kernel, memakai log_writeln untuk pelaporan dan KERNEL_PANIC/kernel_panic_at bila validasi gagal]` | `[sedang — bergantung pada header kernel mcsos/kernel/log.h dan mcsos/kernel/panic.h yang sempat menimbulkan kegagalan build, lihat Bagian 15]` |
| `[include/mcs_sync_selftest.h]` | `[baru]` | `[Deklarasi tunggal void m12_sync_selftest(void) agar dapat dipanggil dari kmain.c]` | `[rendah]` |
| `[tests/m12_sync_host_test.c]` | `[baru]` | `[Host unit test lengkap untuk spinlock, mutex, dan lockdep, dijalankan sebelum objek freestanding dibangun]` | `[sedang]` |
| `[Makefile.m12]` | `[baru]` | `[Target host-test, freestanding, audit, clean; variabel SYNC_SRCS (host) dan KERNEL_SYNC_SRCS (freestanding, menambahkan selftest.c); KERNEL_CFLAGS dengan -Iinclude -Ikernel/include]` | `[tinggi — Makefile.m12 mengalami tiga kelas kegagalan berbeda selama sesi, lihat Bagian 15]` |
| `[kernel/core/kmain.c]` | `[ubah]` | `[Menambahkan pemanggilan m12_sync_selftest() tepat setelah m8_heap_bootstrap() dan sebelum mcsos_scheduler_init(), sehingga selftest sinkronisasi berjalan sebelum scheduler M9 diinisialisasi]` | `[sedang — penempatan pemanggilan memengaruhi urutan log boot]` |
| `[build/mcsos.iso]` | `[baru, artefak]` | `[Image bootable BIOS+UEFI hasil tools/scripts/make_iso.sh memakai Limine dan xorriso, dipakai untuk boot QEMU pada sesi M12 ini karena image sebelumnya belum ada di direktori build/]` | `[rendah]` |
| `[evidence/M12/]` | `[baru]` | `[Seluruh log preflight, build, audit, sha256, dan log serial QEMU sesi M12]` | `[rendah]` |

### 8.3 Ringkasan Diff

```bash
git status --short
git add .
git commit -m "Complete M12 synchronization primitives and lockdep"
git push origin praktikum/m12-sync
```

Output:

```text
[praktikum/m12-sync 673ea92] Complete M12 synchronization primitives and lockdep
 14 files changed, 560 insertions(+), 1 deletion(-)
 create mode 100644 Makefile.m12
 create mode 100644 evidence/M12/m12-sha256-final.txt
 create mode 100644 evidence/M12/nm-undefined.txt
 create mode 100644 evidence/M12/objdump-spinlock.txt
 create mode 100644 evidence/M12/readelf-lockdep.txt
 create mode 100644 evidence/M12/sha256sums.txt
 create mode 100644 include/mcs_sync.h
 create mode 100644 include/mcs_sync_selftest.h
 create mode 100644 kernel/sync/lockdep.c
 create mode 100644 kernel/sync/mutex.c
 create mode 100644 kernel/sync/selftest.c
 create mode 100644 kernel/sync/spinlock.c
 create mode 100644 tests/m12_sync_host_test.c
...
 * [new branch]      praktikum/m12-sync -> praktikum/m12-sync
```

Catatan: `git status --short` sebelum commit menunjukkan `kernel/core/kmain.c` berstatus modified dan seluruh file di atas berstatus untracked; tidak ada file sampah tersisa dari sesi M11 (`M11.txt`, `m11_host_test`, `~$M11.txt`) karena telah dibersihkan pada Langkah 1.

---

## 9. Desain Teknis

### 9.1 Kontrak Primitif Sinkronisasi

```text
Header include/mcs_sync.h mendefinisikan MCS_LOCKDEP_MAX_HELD = 16 (maksimum lock aktif yang dicatat lockdep per konteks) dan MCS_LOCK_NAME_MAX = 32. Kode status memakai konvensi mirip errno POSIX bernilai negatif: MCS_SYNC_OK (0), MCS_SYNC_EINVAL (-22), MCS_SYNC_EBUSY (-16), MCS_SYNC_EPERM (-1), MCS_SYNC_EDEADLK (-35), MCS_SYNC_EOVERFLOW (-75). Tiga struktur data utama: mcs_lockdep_state_t (held_class[16], held_name[16], depth, violation_count), mcs_spinlock_t (locked volatile uint32_t, class_id, name), dan mcs_mutex_t (locked volatile uint32_t, owner uint64_t, class_id, name).
```

### 9.2 Alur Pemanggilan Sinkronisasi

```text
Dua jalur pemanggilan tersedia pada M12: (1) jalur host unit test (tests/m12_sync_host_test.c) yang menautkan langsung lockdep.c, spinlock.c, dan mutex.c tanpa kernel/sync/selftest.c, dijalankan lewat HOSTCC pada Makefile.m12 target host-test; dan (2) jalur kernel nyata lewat kernel/sync/selftest.c yang dipanggil dari kmain() sebagai m12_sync_selftest(), mengeksekusi mcs_spin_init/mcs_spin_lock/mcs_spin_unlock, mcs_mutex_init/mcs_mutex_try_lock/mcs_mutex_unlock, dan mcs_lockdep_init/mcs_lockdep_before_acquire/mcs_lockdep_after_release secara berurutan, melaporkan hasil lewat log_writeln("[M12] sync selftest start"/"passed"), dan memanggil kernel_panic_at bila salah satu validasi gagal. Kedua jalur telah diuji end-to-end pada sesi ini: jalur (1) lewat "make -f Makefile.m12 host-test" ("[PASS] M12 synchronization host tests passed") dan jalur (2) lewat boot QEMU nyata (log "[M12] sync selftest start" dan "[M12] sync selftest passed").
```

### 9.3 Diagram Alur (Deskripsi Tekstual)

```text
[kmain()]
    |
    | m8_heap_bootstrap() selesai
    v
m12_sync_selftest()  (kernel/sync/selftest.c)
    |
    |-- mcs_spin_init / mcs_spin_lock / mcs_spin_unlock / mcs_spin_is_locked
    |-- mcs_mutex_init / mcs_mutex_try_lock / mcs_mutex_unlock / mcs_mutex_is_locked
    |-- mcs_lockdep_init / mcs_lockdep_before_acquire / mcs_lockdep_after_release / mcs_lockdep_is_held
    |
    v
log_writeln("[M12] sync selftest passed")  ATAU  KERNEL_PANIC(...) bila gagal
    |
    v
mcsos_scheduler_init(&g_sched, &g_boot_thread)   (M9, dipanggil setelah selftest M12)
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[mcs_spin_init(lock, class_id, name)]` | `[m12_sync_selftest, host test]` | `[spinlock.c]` | `[lock boleh NULL, fungsi mengecek NULL]` | `[lock->locked=0, class_id dan name tersimpan]` | `[return diam-diam bila lock NULL]` |
| `[mcs_spin_try_lock(lock)]` | `[mcs_spin_lock, kode yang butuh non-blocking acquire]` | `[spinlock.c]` | `[lock sudah diinisialisasi]` | `[return true bila berhasil mengambil lock (xchg lama==0)]` | `[return false bila lock NULL atau sudah terkunci]` |
| `[mcs_spin_lock(lock)]` | `[m12_sync_selftest]` | `[spinlock.c]` | `[lock sudah diinisialisasi]` | `[busy-wait sampai lock berhasil diambil, memakai instruksi pause saat menunggu]` | `[tidak ada batas waktu; berpotensi busy-wait tanpa akhir bila lock tidak pernah dilepas]` |
| `[mcs_spin_unlock(lock)]` | `[m12_sync_selftest]` | `[spinlock.c]` | `[lock sedang dipegang oleh pemanggil]` | `[lock->locked=0 dengan __ATOMIC_RELEASE]` | `[return diam-diam bila lock NULL; tidak ada verifikasi ownership pada level spinlock]` |
| `[mcs_mutex_try_lock(mutex, owner_id)]` | `[m12_sync_selftest]` | `[mutex.c]` | `[mutex sudah diinisialisasi]` | `[mutex terkunci dengan owner=owner_id]` | `[return MCS_SYNC_EBUSY bila sudah terkunci pihak lain]` |
| `[mcs_mutex_unlock(mutex, owner_id)]` | `[m12_sync_selftest]` | `[mutex.c]` | `[mutex sedang dipegang]` | `[mutex terbuka]` | `[return MCS_SYNC_EPERM bila owner_id tidak sesuai pemegang lock]` |
| `[mcs_lockdep_before_acquire(state, class_id, name)]` | `[m12_sync_selftest]` | `[lockdep.c]` | `[state sudah diinisialisasi lewat mcs_lockdep_init]` | `[class_id dicatat pada held_class[depth], depth bertambah]` | `[return MCS_SYNC_EOVERFLOW bila depth sudah mencapai MCS_LOCKDEP_MAX_HELD; violation_count bertambah bila urutan class_id melanggar aturan yang ditetapkan]` |
| `[mcs_lockdep_after_release(state, class_id, name)]` | `[m12_sync_selftest]` | `[lockdep.c]` | `[class_id sedang tercatat sebagai held]` | `[entri class_id dihapus dari held_class, depth berkurang]` | `[return MCS_SYNC_EINVAL bila class_id tidak ditemukan pada daftar held]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `mcs_spinlock_t` `` | `[locked (volatile uint32_t), class_id, name]` | `[Dialokasikan oleh pemanggil (mis. lokal pada m12_sync_selftest atau host test)]` | `[selama kernel/host test berjalan]` | `[locked hanya bernilai 0 atau 1, diakses lewat operasi atomic]` |
| `` `mcs_mutex_t` `` | `[locked, owner (uint64_t), class_id, name]` | `[Dialokasikan oleh pemanggil]` | `[selama kernel/host test berjalan]` | `[owner hanya valid bila locked!=0; unlock oleh owner_id yang salah ditolak MCS_SYNC_EPERM]` |
| `` `mcs_lockdep_state_t` `` | `[held_class[16], held_name[16], depth, violation_count]` | `[Satu instance per konteks/thread, dialokasikan oleh pemanggil]` | `[selama konteks berjalan]` | `[0 <= depth <= MCS_LOCKDEP_MAX_HELD (16); violation_count tidak pernah berkurang]` |

### 9.6 Invariants

Tuliskan invariant yang harus benar sepanjang eksekusi.

1. `Nilai locked pada mcs_spinlock_t dan mcs_mutex_t hanya boleh 0 (terbuka) atau 1 (terkunci), tidak pernah nilai lain, dan hanya diubah lewat operasi atomic (__atomic_exchange_n/__atomic_store_n).`
2. `mcs_mutex_unlock hanya berhasil bila owner_id yang diberikan sama dengan owner yang tercatat saat mutex dikunci; selain itu ditolak dengan MCS_SYNC_EPERM.`
3. `Kedalaman pencatatan lockdep (depth pada mcs_lockdep_state_t) tidak boleh melebihi MCS_LOCKDEP_MAX_HELD (16); percobaan melampaui batas ditolak dengan MCS_SYNC_EOVERFLOW tanpa merusak entri yang sudah tercatat.`
4. `mcs_spin_lock harus menggunakan instruksi PAUSE selama busy-wait agar tidak membebani bus memori secara berlebihan pada CPU dengan hyper-threading.`
5. `m12_sync_selftest() harus memicu KERNEL_PANIC bila salah satu dari validasi spinlock, mutex, atau lockdep gagal, sehingga kegagalan sinkronisasi tidak pernah dibiarkan lolos secara diam-diam pada boot kernel.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[mcs_spinlock_t individual]` | `[Pemanggil yang mengalokasikan lock tersebut]` | `[dirinya sendiri (self-protecting lewat atomic)]` | `[Ya, dirancang aman dipakai tanpa tidur]` | `[Tidak ada proteksi terhadap penggunaan sebelum mcs_spin_init dipanggil]` |
| `[mcs_mutex_t individual]` | `[Pemanggil yang mengalokasikan mutex tersebut]` | `[dirinya sendiri]` | `[Belum diverifikasi aman untuk interrupt context karena mutex dirancang untuk critical section yang lebih panjang]` | `[Belum ada mekanisme priority inheritance]` |
| `[mcs_lockdep_state_t per konteks]` | `[Konteks/thread yang memilikinya]` | `[none — murni pembukuan, tidak melindungi data lain]` | `[Ya]` | `[Baru mendeteksi dan mencatat pelanggaran (violation_count), belum mengambil tindakan pemulihan otomatis]` |

Lock order yang berlaku:

```text
Pada sesi M12 ini belum ada kebijakan lock ordering global lintas-subsistem yang didaftarkan secara formal ke lockdep (mis. urutan wajib scheduler-lock sebelum heap-lock); m12_sync_selftest() hanya membuktikan bahwa mekanisme pencatatan dan deteksi pelanggaran lockdep berfungsi pada skenario uji internal, bukan menegakkan kebijakan lock order lintas subsistem MCSOS secara keseluruhan.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Pointer NULL pada seluruh fungsi spinlock/mutex]` | `[spinlock.c, mutex.c]` | `[Setiap fungsi mengecek if (lock == 0) / if (mutex == 0) sebelum mendereferensi]` | `[Isi kernel/sync/spinlock.c: mcs_spin_init, mcs_spin_try_lock, mcs_spin_unlock, mcs_spin_is_locked seluruhnya diawali pemeriksaan NULL]` |
| `[Busy-wait tanpa batas waktu pada mcs_spin_lock]` | `[mcs_spin_lock]` | `[Belum ada timeout; dimitigasi hanya lewat disiplin pemanggil untuk selalu memanggil mcs_spin_unlock]` | `[Isi kernel/sync/spinlock.c, dicatat sebagai known issue Bagian 20]` |
| `[Overflow pencatatan lockdep bila depth > MCS_LOCKDEP_MAX_HELD]` | `[mcs_lockdep_before_acquire]` | `[Dibatasi lewat pengecekan depth dan pengembalian MCS_SYNC_EOVERFLOW]` | `[Kontrak pada include/mcs_sync.h]` |
| `[Header lintas-lapisan tidak ditemukan saat kompilasi freestanding selftest.c]` | `[kernel/sync/selftest.c vs Makefile.m12]` | `[Menambahkan flag -Ikernel/include pada KERNEL_CFLAGS Makefile.m12]` | `[Log build sebelum dan sesudah perbaikan, lihat Bagian 15]` |
| `[selftest.c salah dimasukkan ke daftar sumber host-test]` | `[Makefile.m12 SYNC_SRCS]` | `[Memisahkan SYNC_SRCS (host) dari KERNEL_SYNC_SRCS = SYNC_SRCS + selftest.c (freestanding)]` | `[Log build evidence/M12/m12-build-after-selftest.log]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[owner_id pada mcs_mutex_unlock]` | `[Nilai owner_id yang diberikan pemanggil]` | `[Dibandingkan dengan owner yang tercatat saat lock diambil]` | `[Return MCS_SYNC_EPERM, mutex tidak dibuka oleh pihak yang bukan pemiliknya]` |
| `[class_id pada mcs_lockdep_before_acquire]` | `[Nomor kelas lock yang diberikan pemanggil]` | `[Dicocokkan terhadap daftar held_class yang sudah tercatat untuk mendeteksi pelanggaran urutan]` | `[violation_count bertambah, fungsi tetap mengembalikan status tanpa crash]` |
| `[Build system Makefile.m12]` | `[Target M12 yang ditambahkan manual dan diedit berulang kali lewat nano]` | `[Uji make -f Makefile.m12 host-test dan make -f Makefile.m12 audit sebelum diintegrasikan ke build kernel penuh]` | `[Build dihentikan bila host unit test sinkronisasi gagal atau assertion audit (grep -q) tidak terpenuhi]` |

---

## 10. Langkah Kerja Implementasi

Gunakan tabel berikut untuk setiap langkah. Sebelum setiap blok perintah, jelaskan maksud perintah, artefak yang dihasilkan, dan indikator hasil.

### Langkah 1 — `Preflight, Pembersihan Sisa Sesi M11, dan Baseline Toolchain`

Maksud langkah:

```text
Memverifikasi direktori kerja, branch aktif (praktikum-m11-elf-user-loader), status git, dan versi toolchain sebelum memulai M12. Ditemukan tiga file sampah untracked peninggalan sesi M11 (M11.txt, m11_host_test, ~$M11.txt) yang perlu dibersihkan terlebih dahulu.
```

Perintah:

```bash
cd ~/src/mcsos
pwd
git status --short
git branch --show-current
git log --oneline -5
ls -lh M11.txt m11_host_test '~$M11.txt'
rm -f M11.txt m11_host_test '~$M11.txt'
git status --short
mkdir -p evidence/M12
{ date -Is; uname -a; clang --version | head -n 1; cc --version | head -n 1; make --version | head -n 1; git rev-parse --short HEAD; git status --short; } | tee evidence/M12/preflight.log
```

Output ringkas:

```text
/home/syifa/src/mcsos
praktikum-m11-elf-user-loader
a701faa (HEAD -> praktikum-m11-elf-user-loader, origin/praktikum-m11-elf-user-loader) Complete M11 ELF user loader integration
M11.txt (0 byte), m11_host_test (33K), ~$M11.txt (162 byte) berhasil dihapus
git status --short kosong setelah rm
2026-07-07T20:30:30+07:00
Linux WIN-E2QNIIEGDH4 6.18.33.2-microsoft-standard-WSL2
Ubuntu clang version 21.1.8 (6ubuntu1)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Working tree bersih dari file sampah M11]` | `[~/src/mcsos]` | `[Baseline bersih sebelum branch M12 dibuka]` |
| `[preflight.log]` | `[evidence/M12/preflight.log]` | `[Bukti versi toolchain dan commit awal sebelum perubahan M12]` |

Indikator berhasil:

```text
File sampah peninggalan M11 berhasil dihapus, working tree bersih, dan seluruh versi toolchain tercatat pada evidence/M12/preflight.log sebelum branch M12 dibuka.
```

### Langkah 2 — `Membuka Branch Baru dan Menyusun Struktur Direktori Sinkronisasi`

Maksud langkah:

```text
Membuat branch praktikum/m12-sync dari branch praktikum-m11-elf-user-loader, lalu menyiapkan direktori kerja untuk header, implementasi, test, dan skrip M12.
```

Perintah:

```bash
git checkout -b praktikum/m12-sync
mkdir -p include kernel/sync tests scripts evidence/M12
git branch --show-current
```

Output ringkas:

```text
Switched to a new branch 'praktikum/m12-sync'
praktikum/m12-sync
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Branch kerja baru]` | `[praktikum/m12-sync]` | `[Isolasi pengerjaan M12 dari branch M11]` |
| `[Direktori kernel/sync]` | `[kernel/sync/]` | `[Lokasi implementasi spinlock, mutex, lockdep, dan selftest]` |

Indikator berhasil:

```text
Branch praktikum/m12-sync aktif dan struktur direktori kernel/sync siap dipakai untuk file implementasi berikutnya.
```

### Langkah 3 — `Menulis Header Kontrak Sinkronisasi`

Maksud langkah:

```text
Menulis include/mcs_sync.h berisi kode status MCS_SYNC_*, struktur mcs_lockdep_state_t, mcs_spinlock_t, mcs_mutex_t, dan deklarasi seluruh fungsi lockdep, spinlock, dan mutex.
```

Perintah:

```bash
nano include/mcs_sync.h
ls -l include/mcs_sync.h
sed -n '1,200p' include/mcs_sync.h
```

Output ringkas:

```text
-rw-r--r-- 1 syifa syifa 1799 Jul  7 20:32 include/mcs_sync.h
#define MCS_LOCKDEP_MAX_HELD 16u
#define MCS_LOCK_NAME_MAX 32u
#define MCS_SYNC_OK 0
#define MCS_SYNC_EINVAL (-22)
#define MCS_SYNC_EBUSY (-16)
#define MCS_SYNC_EPERM (-1)
#define MCS_SYNC_EDEADLK (-35)
#define MCS_SYNC_EOVERFLOW (-75)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[mcs_sync.h]` | `[include/mcs_sync.h]` | `[Kontrak sinkronisasi lengkap, 1799 byte]` |

Indikator berhasil:

```text
Header selesai ditulis dengan seluruh struct, enumerasi kode status, dan deklarasi fungsi yang diperlukan oleh lockdep.c, spinlock.c, dan mutex.c.
```

### Langkah 4 — `Menulis lockdep.c, spinlock.c, dan mutex.c`

Maksud langkah:

```text
Menulis tiga file implementasi inti: kernel/sync/lockdep.c (pembukuan dan validasi urutan lock), kernel/sync/spinlock.c (spinlock atomic berbasis xchg dan pause), dan kernel/sync/mutex.c (mutex ber-owner).
```

Perintah:

```bash
nano kernel/sync/lockdep.c
ls -l kernel/sync/lockdep.c
nano kernel/sync/spinlock.c
ls -l kernel/sync/spinlock.c
nano kernel/sync/mutex.c
ls -l kernel/sync/mutex.c
sed -n '1,200p' kernel/sync/spinlock.c
```

Output ringkas:

```text
-rw-r--r-- 1 syifa syifa 2041 Jul  7 20:34 kernel/sync/lockdep.c
-rw-r--r-- 1 syifa syifa 1178 Jul  7 20:35 kernel/sync/spinlock.c
-rw-r--r-- 1 syifa syifa 1620 Jul  7 20:37 kernel/sync/mutex.c

void mcs_spin_init(mcs_spinlock_t *lock, uint32_t class_id, const char *name) {
    if (lock == 0) { return; }
    __atomic_store_n(&lock->locked, 0u, __ATOMIC_RELAXED);
    lock->class_id = class_id;
    lock->name = name;
}
bool mcs_spin_try_lock(mcs_spinlock_t *lock) {
    if (lock == 0) { return false; }
    uint32_t old = __atomic_exchange_n(&lock->locked, 1u, __ATOMIC_ACQUIRE);
    return old == 0u;
}
void mcs_spin_lock(mcs_spinlock_t *lock) {
    while (!mcs_spin_try_lock(lock)) {
        while (__atomic_load_n(&lock->locked, __ATOMIC_RELAXED) != 0u) {
            mcs_cpu_relax();
        }
    }
}
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[lockdep.c]` | `[kernel/sync/lockdep.c]` | `[Implementasi lock-dependency validator, 2041 byte]` |
| `[spinlock.c]` | `[kernel/sync/spinlock.c]` | `[Implementasi spinlock atomic, 1178 byte]` |
| `[mutex.c]` | `[kernel/sync/mutex.c]` | `[Implementasi mutex ber-owner, 1620 byte]` |

Indikator berhasil:

```text
Ketiga file selesai ditulis; kompilasi dan pengujian sintaks diverifikasi pada langkah host test berikutnya.
```

### Langkah 5 — `Menyusun Host Unit Test dan Makefile.m12`

Maksud langkah:

```text
Menulis tests/m12_sync_host_test.c untuk menguji spinlock, mutex, dan lockdep, serta Makefile.m12 dengan target host-test, freestanding, audit, dan clean.
```

Perintah:

```bash
nano tests/m12_sync_host_test.c
ls -l tests/m12_sync_host_test.c
nano Makefile.m12
ls -l Makefile.m12
make -f Makefile.m12 clean
make -f Makefile.m12 all CC=clang | tee evidence/M12/m12-build.log
```

Output ringkas:

```text
-rw-r--r-- 1 syifa syifa 3264 Jul  7 20:38 tests/m12_sync_host_test.c
-rw-r--r-- 1 syifa syifa 1371 Jul  7 20:39 Makefile.m12
rm -rf build
cc -std=c17 -Wall -Wextra -Werror -Iinclude -O2 -pthread kernel/sync/lockdep.c kernel/sync/spinlock.c kernel/sync/mutex.c tests/m12_sync_host_test.c -o build/m12/m12_sync_host_test
build/m12/m12_sync_host_test | tee build/m12/host-test.log
[PASS] M12 synchronization host tests passed
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m12_sync_host_test.c]` | `[tests/m12_sync_host_test.c]` | `[Host unit test sinkronisasi, 3264 byte]` |
| `[Makefile.m12]` | `[Makefile.m12]` | `[Build system khusus M12, 1371 byte]` |
| `[build/m12/m12_sync_host_test]` | `[build/m12/]` | `[Binari host test]` |
| `[evidence/M12/m12-build.log]` | `[evidence/M12/]` | `[Log build dan hasil host test pertama]` |

Indikator berhasil:

```text
Host unit test lulus dengan pesan "[PASS] M12 synchronization host tests passed" pada percobaan pertama tanpa perlu perbaikan.
```

### Langkah 6 — `Kompilasi Freestanding dan Audit Objek Inti Sinkronisasi`

Maksud langkah:

```text
Mengompilasi lockdep.c, spinlock.c, dan mutex.c sebagai objek freestanding x86_64-elf, lalu mengaudit hasilnya dengan nm, readelf, objdump, dan sha256sum.
```

Perintah:

```bash
clang -std=c17 -Wall -Wextra -Werror -Iinclude -target x86_64-elf -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -mno-red-zone -O2 -c kernel/sync/lockdep.c -o build/m12/lockdep.o
clang ... -c kernel/sync/spinlock.c -o build/m12/spinlock.o
clang ... -c kernel/sync/mutex.c -o build/m12/mutex.o
nm -u build/m12/lockdep.o build/m12/spinlock.o build/m12/mutex.o | tee evidence/M12/nm-undefined.txt
readelf -h build/m12/lockdep.o | tee evidence/M12/readelf-lockdep.txt
objdump -d build/m12/spinlock.o | tee evidence/M12/objdump-spinlock.txt
sha256sum build/m12/lockdep.o build/m12/spinlock.o build/m12/mutex.o build/m12/m12_sync_host_test > evidence/M12/sha256sums.txt
```

Output ringkas:

```text
build/m12/lockdep.o:  (kosong, tidak ada simbol undefined)
build/m12/spinlock.o: (kosong)
build/m12/mutex.o:    (kosong)

ELF Header build/m12/lockdep.o:
  Class: ELF64
  Machine: Advanced Micro Devices X86-64
  Type: REL (Relocatable file)

objdump -d build/m12/spinlock.o menunjukkan fungsi mcs_spin_init, mcs_spin_try_lock (xchg %eax,(%rdi)), mcs_spin_lock (pause), mcs_spin_unlock, mcs_spin_is_locked.

33ec8f024ce9cdcc3de3c7b77f009ee1c27c1ffc1286fbe77e8b6e53cea1ae0d  build/m12/lockdep.o
e31f45d64eda89034651cb6b421cebf35d5b88b83af9fa49777b2cb036bb3415  build/m12/spinlock.o
384e6e12412a02a95dc2606064c894adb8a52267f3ef7a3a66bf6d4afff3f4b3  build/m12/mutex.o
fb42f00b52cd116769d9049cf0d2f48f2b0c2f9cf283cb439b90a2597766a3f6  build/m12/m12_sync_host_test
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[lockdep.o, spinlock.o, mutex.o]` | `[build/m12/]` | `[Objek freestanding ELF64 x86_64]` |
| `[nm-undefined.txt, readelf-lockdep.txt, objdump-spinlock.txt, sha256sums.txt]` | `[evidence/M12/]` | `[Bukti audit objek freestanding]` |

Indikator berhasil:

```text
Ketiga objek freestanding berhasil dikompilasi tanpa error/warning, tidak ada simbol undefined (nm -u kosong untuk ketiganya), struktur ELF64 x86_64 terverifikasi lewat readelf, dan disassembly objdump menunjukkan instruksi atomic (xchg, pause) sesuai desain.
```

### Langkah 7 — `Investigasi Struktur Kernel Existing untuk Integrasi`

Maksud langkah:

```text
Menelusuri kernel/core untuk memahami pola pemanggilan kmain(), fungsi logging (log_writeln dkk.), dan mekanisme panic (kernel_panic_at, KERNEL_PANIC) sebelum menulis selftest.c dan mengintegrasikannya ke kmain.c, agar konvensi yang dipakai konsisten dengan milestone M4/M5/M8/M9/M10/M11 sebelumnya.
```

Perintah:

```bash
grep -R "void kmain" -n kernel
grep -R "panic" -n kernel/core kernel/include include
grep -R "log_" -n kernel/core kernel/include include
find kernel -name "*.c" | sort
sed -n '228,330p' kernel/core/kmain.c
```

Output ringkas:

```text
kernel/core/kmain.c:228:void kmain(void)
kernel/core/panic.c: kernel_panic_at, KERNEL_PANIC(reason, code)
kernel/include/mcsos/kernel/log.h: log_init, log_putc, log_write, log_writeln, log_hex64, log_key_value_hex64

kernel/arch/x86_64/idt.c, kernel/core/kmain.c, kernel/core/log.c, kernel/core/panic.c,
kernel/core/pic.c, kernel/core/pit.c, kernel/core/pmm.c, kernel/core/serial.c, kernel/core/trap.c,
kernel/core/vmm.c, kernel/lib/memory.c, kernel/mcsos_thread.c, kernel/mm/kmem.c,
kernel/sync/lockdep.c, kernel/sync/mutex.c, kernel/sync/spinlock.c,
kernel/syscall/syscall.c, kernel/user/m11_elf_loader.c
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Pemahaman struktur kmain() dan urutan boot M5–M11]` | `[kernel/core/kmain.c]` | `[Dasar penentuan titik penyisipan m12_sync_selftest()]` |

Indikator berhasil:

```text
Ditemukan titik penyisipan yang tepat: setelah m8_heap_bootstrap() (kernel heap siap) dan sebelum mcsos_scheduler_init() (M9), sehingga selftest sinkronisasi berjalan sebelum scheduler diaktifkan.
```

### Langkah 8 — `Menulis kernel/sync/selftest.c dan Mengintegrasikannya ke kmain()`

Maksud langkah:

```text
Menulis kernel/sync/selftest.c berisi m12_sync_selftest() yang mengeksekusi spinlock, mutex, dan lockdep secara nyata, membuat include/mcs_sync_selftest.h sebagai deklarasi, lalu menyisipkan pemanggilan m12_sync_selftest() ke kmain.c.
```

Perintah:

```bash
nano kernel/sync/selftest.c
wc -l kernel/sync/selftest.c
nano include/mcs_sync_selftest.h
cat include/mcs_sync_selftest.h
nano kernel/core/kmain.c
grep -n "m12_sync_selftest" kernel/core/kmain.c
```

Output ringkas:

```text
53 kernel/sync/selftest.c
#ifndef MCS_SYNC_SELFTEST_H
#define MCS_SYNC_SELFTEST_H
void m12_sync_selftest(void);
#endif
262:m12_sync_selftest();
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[selftest.c]` | `[kernel/sync/selftest.c]` | `[Selftest sinkronisasi kernel nyata, 53 baris]` |
| `[mcs_sync_selftest.h]` | `[include/mcs_sync_selftest.h]` | `[Deklarasi m12_sync_selftest]` |
| `[kmain.c termodifikasi]` | `[kernel/core/kmain.c baris 262]` | `[Pemanggilan m12_sync_selftest() setelah m8_heap_bootstrap()]` |

Indikator berhasil:

```text
Pemanggilan m12_sync_selftest() berhasil disisipkan tepat setelah m8_heap_bootstrap() dan sebelum mcsos_scheduler_init(), diverifikasi lewat grep pada baris 262 kmain.c.
```

### Langkah 9 — `Debugging Makefile.m12: Tiga Kelas Kegagalan`

Maksud langkah:

```text
Memperbarui Makefile.m12 agar SYNC_SRCS (host test) tetap tidak menyertakan selftest.c, sementara KERNEL_SYNC_SRCS (freestanding) menyertakannya, lalu membangun ulang. Proses ini menemukan tiga kegagalan berurutan yang dianalisis lengkap pada Bagian 15.
```

Perintah:

```bash
nano Makefile.m12
make -f Makefile.m12 clean
make -f Makefile.m12 all CC=clang | tee evidence/M12/m12-build-after-selftest.log
```

Output ringkas (kegagalan pertama):

```text
kernel/sync/selftest.c:2:10: fatal error: mcsos/kernel/log.h: No such file or directory
make: *** [Makefile.m12:20: host-test] Error 1
```

Output ringkas (kegagalan kedua, setelah Makefile.m12 diedit):

```text
Makefile.m12:29: *** missing separator (did you mean TAB instead of 8 spaces?).  Stop.
```

Output ringkas (kegagalan ketiga, setelah separator diperbaiki):

```text
kernel/sync/selftest.c:2:10: fatal error: 'mcsos/kernel/log.h' file not found
make: *** [Makefile.m12:29: freestanding] Error 1
```

Output ringkas (setelah -Ikernel/include ditambahkan ke KERNEL_CFLAGS):

```text
clang ... -Iinclude -Ikernel/include -target x86_64-elf ... -c kernel/sync/selftest.c -o build/m12/selftest.o
nm -u build/m12/lockdep.o build/m12/spinlock.o build/m12/mutex.o build/m12/selftest.o | tee build/m12/nm-undefined.txt
build/m12/selftest.o:
                 U kernel_panic_at
                 U log_writeln
                 U mcs_lockdep_init
                 U mcs_mutex_init / mcs_mutex_is_locked / mcs_mutex_try_lock / mcs_mutex_unlock
                 U mcs_spin_init / mcs_spin_is_locked / mcs_spin_lock / mcs_spin_unlock
make: *** [Makefile.m12:36: audit] Error 1
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Makefile.m12 final]` | `[Makefile.m12]` | `[SYNC_SRCS terpisah dari KERNEL_SYNC_SRCS, KERNEL_CFLAGS memuat -Ikernel/include, dan target audit hanya memeriksa nm -u pada lockdep.o/spinlock.o/mutex.o (tidak menyertakan selftest.o yang secara sah masih memiliki simbol undefined sebelum ditautkan ke kernel penuh)]` |
| `[evidence/M12/m12-build-after-selftest.log, m12-build-final.log]` | `[evidence/M12/]` | `[Log build sebelum dan sesudah seluruh perbaikan]` |

Indikator berhasil:

```text
Setelah tiga perbaikan berurutan (pemisahan SYNC_SRCS/KERNEL_SYNC_SRCS, penormalan separator resep, dan penambahan -Ikernel/include), "make -f Makefile.m12 all CC=clang" berhasil membangun host test, seluruh objek freestanding (termasuk selftest.o), dan target audit lulus tanpa error, dengan simbol undefined pada selftest.o dipahami sebagai hal yang wajar karena akan diresolusi saat ditautkan ke build/kernel.elf.
```

### Langkah 10 — `Build Kernel Penuh dan Audit ELF`

Maksud langkah:

```text
Membangun kernel penuh (build/kernel.elf) dengan modul sinkronisasi M12 terintegrasi, memakai Makefile utama proyek (bukan Makefile.m12), lalu memverifikasi hasil audit bawaan.
```

Perintah:

```bash
make clean
make all 2>&1 | tee evidence/M12/qemu/kernel-build.log
```

Output ringkas:

```text
clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding ... -c kernel/sync/lockdep.c -o build/normal/kernel/sync/lockdep.o
clang ... -c kernel/sync/mutex.c -o build/normal/kernel/sync/mutex.o
clang ... -c kernel/sync/selftest.c -o build/normal/kernel/sync/selftest.o
clang ... -c kernel/sync/spinlock.c -o build/normal/kernel/sync/spinlock.o
ld.lld -nostdlib -static -z max-page-size=0x1000 -T linker.ld -Map=build/kernel.map -o build/kernel.elf ... build/normal/kernel/sync/lockdep.o build/normal/kernel/sync/mutex.o build/normal/kernel/sync/selftest.o build/normal/kernel/sync/spinlock.o ...
grep -q 'ELF64' build/kernel.readelf.header.txt
grep -q 'Machine:[[:space:]]*Advanced Micro Devices X86-64' build/kernel.readelf.header.txt
grep -q 'kmain' build/kernel.syms.txt
grep -q 'x86_64_idt_init' build/kernel.syms.txt
grep -q 'x86_64_trap_dispatch' build/kernel.syms.txt
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[build/kernel.elf]` | `[build/kernel.elf, 43K]` | `[Kernel penuh dengan modul sinkronisasi M12 tertaut]` |
| `[build/kernel.readelf.header.txt, kernel.syms.txt, kernel.disasm.txt]` | `[build/]` | `[Bukti audit ELF64 x86_64 dan simbol kernel]` |

Indikator berhasil:

```text
Seluruh assertion audit bawaan (ELF64, Machine X86-64, kmain, x86_64_idt_init, x86_64_trap_dispatch, iretq, lidt) lulus, dan build/kernel.elf berhasil dibangun sebesar 43K dengan objek lockdep.o/mutex.o/selftest.o/spinlock.o tertaut tanpa simbol undefined tersisa.
```

### Langkah 11 — `Membuat Ulang Image ISO dan Menjalankan Boot QEMU`

Maksud langkah:

```text
Membuat image bootable build/mcsos.iso karena belum tersedia di direktori build/ pada sesi ini, lalu menjalankan smoke test QEMU dan memverifikasi log serial memuat penanda M12.
```

Perintah:

```bash
ls -lh build/mcsos.iso   # belum ada
bash tools/scripts/make_iso.sh
ls -lh build/mcsos.iso
scripts/m11_qemu_smoke.sh build/mcsos.iso evidence/M12/qemu/m12_serial.log
cat evidence/M12/qemu/m12_serial.log
```

Output ringkas:

```text
ISO image produced: 2118 sectors
e10dd97a998b3fc95f6bec725d4ead1b4d0b40b71593f6f2d6a375add6d84e74  build/mcsos.iso
OK: ISO dibuat pada build/mcsos.iso
-rw-r--r-- 1 syifa syifa 4.2M Jul  7 21:30 build/mcsos.iso

qemu-system-x86_64: terminating on signal 15 from pid 3620 (timeout)
[OK] log M11 terdeteksi di evidence/M12/qemu/m12_serial.log

limine: Loading executable `boot():/boot/kernel.elf`...
[MCSOS:M5] boot: external interrupt bring-up start
...
[M8] kernel heap initialized
[M12] sync selftest start
[M12] sync selftest passed
[M10] user region initialized
[M10] syscall subsystem initialized
[M10] syscall ping ok
[M10] syscall get_ticks ok
[M10] syscall smoke done
[M11] before call
[M11] smoke entered
[M11] elf: plan ok
[M11] entry=0x0000000000401000 segments=0x0000000000000001
[M11] user image plan ready
[M11] after call
[M9] scheduler initialized
[MCSOS:M5] sti: enabling interrupts
[MCSOS:TIMER] ticks=0x0000000000000064
...
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[build/mcsos.iso]` | `[build/mcsos.iso, 4.2M]` | `[Image bootable BIOS/UEFI dengan kernel M12 terintegrasi]` |
| `[evidence/M12/qemu/m12_serial.log]` | `[evidence/M12/qemu/]` | `[Log serial boot QEMU nyata]` |

Indikator berhasil:

```text
Log serial memuat "[M12] sync selftest start" dan "[M12] sync selftest passed" tepat setelah "[M8] kernel heap initialized" dan sebelum "[M10] user region initialized", membuktikan m12_sync_selftest() benar-benar berjalan dan lulus pada boot kernel nyata sebelum subsistem M9/M10/M11 diinisialisasi.
```

### Langkah 12 — `Sesi Debugging Remote GDB`

Maksud langkah:

```text
Menjalankan QEMU dalam mode berhenti-di-awal (-s -S) dan menyambungkan GDB untuk memasang breakpoint pada m12_sync_selftest, mcs_spin_lock, dan mcs_lockdep_before_acquire guna memverifikasi jalur eksekusi primitif sinkronisasi pada kernel nyata.
```

Perintah:

```bash
qemu-system-x86_64 -machine q35 -m 512M \
  -serial file:evidence/M12/qemu/gdb_serial.log -s -S \
  -no-reboot -no-shutdown -cdrom build/mcsos.iso &
gdb build/kernel.elf
(gdb) target remote localhost:1234
(gdb) info address m12_sync_selftest
(gdb) info address mcs_spin_lock
(gdb) break m12_sync_selftest
(gdb) break mcs_spin_lock
(gdb) break mcs_lockdep_before_acquire
(gdb) continue
```

Output ringkas:

```text
Reading symbols from build/kernel.elf...
(No debugging symbols found in build/kernel.elf)
Symbol "m12_sync_selftest" is at 0xffffffff80004830 in a file compiled without debugging.
Symbol "mcs_spin_lock" is at 0xffffffff80004a40 in a file compiled without debugging.
Breakpoint 1 at 0xffffffff80004834
Breakpoint 2 at 0xffffffff80004a48
Breakpoint 3 at 0xffffffff800043d8
Continuing.
Program received signal SIGINT, Interrupt.
0xffffffff80000975 in cpu_hlt ()
(gdb) bt
#0  0xffffffff80000975 in cpu_hlt ()
#1  0xffffffff80000457 in kmain ()
(gdb) disassemble /m mcs_spin_lock
... (disassembly statis fungsi berhasil ditampilkan lewat tabel simbol ELF)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Sesi debugging interaktif]` | `[terminal]` | `[Verifikasi alamat simbol m12_sync_selftest, mcs_spin_lock, mcs_lockdep_before_acquire pada build/kernel.elf]` |
| `[evidence/M12/gdb-session.log]` | `[evidence/M12/, 0 byte]` | `[Percobaan perekaman sesi lewat perintah "script"; lihat catatan indikator di bawah]` |

Indikator berhasil (dengan catatan):

```text
Alamat ketiga simbol berhasil diverifikasi lewat "info address" dan breakpoint berhasil dipasang (Breakpoint 1/2/3 diterima GDB tanpa error). Namun demikian, setelah "continue" dijalankan, sesi berakhir dengan interupsi manual (Ctrl+C) dan backtrace menunjukkan eksekusi sudah berada pada cpu_hlt() di dalam loop tak berhingga kmain() (setelah "[MCSOS:M5] sti: enabling interrupts"), bukan berhenti tepat pada salah satu dari ketiga breakpoint yang dipasang. Dengan kata lain, breakpoint tidak terbukti benar-benar tertangkap pada sesi ini — hasil debugging bersifat inconclusive dan dicatat sebagai known issue pada Bagian 15 dan 20, bukan diklaim sebagai bukti breakpoint berhasil. Percobaan kedua merekam sesi lewat perintah "script evidence/M12/gdb-session.log" juga tidak berhasil menangkap ulang interaksi breakpoint tersebut (file log tersimpan kosong, 0 byte).
```

### Langkah 13 — `Pengumpulan Bukti Akhir, Commit, dan Push`

Maksud langkah:

```text
Mengumpulkan hash SHA-256 akhir seluruh artefak penting, menyalin log serial final ke evidence/M12/, memeriksa status git, lalu melakukan commit dan push ke remote.
```

Perintah:

```bash
cp evidence/M12/qemu/m12_serial.log evidence/M12/m12-qemu-serial-final.log
sha256sum build/normal/kernel/sync/lockdep.o build/normal/kernel/sync/spinlock.o \
  build/normal/kernel/sync/mutex.o build/normal/kernel/sync/selftest.o \
  build/kernel.elf build/mcsos.iso > evidence/M12/m12-sha256-final.txt
nm build/kernel.elf | grep m12
nm build/kernel.elf | grep mcs_spin
git status
git add .
git commit -m "Complete M12 synchronization primitives and lockdep"
git push origin praktikum/m12-sync
```

Output ringkas:

```text
ffffffff80004830 T m12_sync_selftest
ffffffff800049a0 T mcs_spin_init
ffffffff800049f0 T mcs_spin_try_lock
ffffffff80004a40 T mcs_spin_lock
ffffffff80004a90 T mcs_spin_unlock
ffffffff80004ac0 T mcs_spin_is_locked

[praktikum/m12-sync 673ea92] Complete M12 synchronization primitives and lockdep
 14 files changed, 560 insertions(+), 1 deletion(-)
 * [new branch]      praktikum/m12-sync -> praktikum/m12-sync
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[evidence/M12/m12-sha256-final.txt, m12-qemu-serial-final.log]` | `[evidence/M12/]` | `[Bukti hash akhir dan log serial final]` |
| `[Commit 673ea92]` | `[branch praktikum/m12-sync]` | `[Snapshot akhir seluruh pekerjaan M12]` |

Indikator berhasil:

```text
Seluruh simbol m12_sync_selftest dan mcs_spin_* ditemukan pada build/kernel.elf dengan alamat yang konsisten dengan sesi GDB sebelumnya, commit 673ea92 berhasil dibuat dengan 14 file berubah, dan branch praktikum/m12-sync berhasil dipush ke remote GitHub.
```

---

## 11. Checkpoint Buildable

Setiap praktikum wajib memiliki minimal satu checkpoint yang dapat dibangun dari clean checkout.

| Checkpoint | Perintah | Expected result | Status |
|---|---|---|---|
| C1 | `` `make -f Makefile.m12 clean && make -f Makefile.m12 all CC=clang` `` (host-test) | `[PASS] M12 synchronization host tests passed` | `[PASS]` |
| C2 | `` `clang --target=x86_64-elf -ffreestanding -c kernel/sync/lockdep.c -c kernel/sync/spinlock.c -c kernel/sync/mutex.c` `` | `[Objek freestanding berhasil dibangun tanpa error]` | `[PASS]` |
| C3 | `` `make -f Makefile.m12 audit` `` | `[nm -u lockdep.o/spinlock.o/mutex.o kosong, readelf ELF64 X86-64]` | `[PASS setelah tiga perbaikan Makefile.m12]` |
| C4 | `` `make clean && make all` `` (Makefile utama, build kernel penuh) | `[build/kernel.elf berhasil dibangun dengan modul sync M12 terintegrasi]` | `[PASS]` |
| C5 | `` `bash tools/scripts/make_iso.sh` `` | `[build/mcsos.iso berhasil dibuat dengan Limine bios-install]` | `[PASS]` |
| C6 | `` `scripts/m11_qemu_smoke.sh build/mcsos.iso evidence/M12/qemu/m12_serial.log` `` | `[Log serial memuat "[M12] sync selftest start" dan "[M12] sync selftest passed"]` | `[PASS]` |
| C7 | `` `gdb build/kernel.elf` + `target remote localhost:1234` + breakpoint `` | `[Breakpoint pada m12_sync_selftest/mcs_spin_lock/mcs_lockdep_before_acquire benar-benar tertangkap]` | `[INCONCLUSIVE — lihat Bagian 12.4 dan 15]` |
| C8 | `` `git commit` dan `git push` `` | `[Commit 673ea92 berhasil dibuat dan dipush]` | `[PASS]` |

Catatan checkpoint:

```text
Seluruh checkpoint inti M12 berhasil dilewati: header dan implementasi sinkronisasi valid secara sintaks, host unit test lulus, objek freestanding lockdep/spinlock/mutex teraudit tanpa simbol undefined, kernel penuh berhasil dibangun dengan modul sync terintegrasi, image ISO berhasil dibuat ulang, dan boot QEMU nyata membuktikan m12_sync_selftest() berjalan dan lulus sebelum scheduler M9 diinisialisasi. Satu-satunya checkpoint yang tidak dapat dinyatakan PASS penuh adalah C7 (sesi debugging GDB): breakpoint berhasil dipasang tetapi tidak terbukti tertangkap pada continue, sehingga statusnya dicatat sebagai inconclusive, bukan PASS maupun FAIL murni.
```

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

```bash
make -f Makefile.m12 clean
make -f Makefile.m12 all CC=clang
```

Hasil:

```text
[PASS] M12 synchronization host tests passed
(build/m12/lockdep.o, spinlock.o, mutex.o, selftest.o berhasil dibangun, seluruh grep -q assertion audit lulus)
```

Status: `[PASS]`

### 12.2 Static Inspection

```bash
nm -u build/m12/lockdep.o build/m12/spinlock.o build/m12/mutex.o
readelf -h build/m12/lockdep.o
objdump -d build/m12/spinlock.o
nm build/kernel.elf | grep m12
nm build/kernel.elf | grep mcs_spin
```

Hasil penting:

```text
evidence/M12/nm-undefined.txt kosong untuk lockdep.o/spinlock.o/mutex.o (tidak ada simbol undefined)
ELF64, Machine: Advanced Micro Devices X86-64, Type: REL
Fungsi mcs_spin_init, mcs_spin_try_lock, mcs_spin_lock, mcs_spin_unlock, mcs_spin_is_locked ditemukan pada build/m12/spinlock.o dengan instruksi xchg dan pause
m12_sync_selftest ditemukan pada build/kernel.elf alamat 0xffffffff80004830
```

Status: `[PASS]`

### 12.3 QEMU Smoke Test

```bash
bash tools/scripts/make_iso.sh
scripts/m11_qemu_smoke.sh build/mcsos.iso evidence/M12/qemu/m12_serial.log
cat evidence/M12/qemu/m12_serial.log
```

Hasil:

```text
Boot berhasil lewat Limine. Log serial memuat "[M8] kernel heap initialized", "[M12] sync selftest start", "[M12] sync selftest passed", diikuti "[M10] user region initialized" dan seterusnya sampai "[M9] scheduler initialized" dan tick timer berjalan normal.
```

Status: `[PASS]`

### 12.4 GDB Debug Evidence

```bash
qemu-system-x86_64 -machine q35 -m 512M -serial file:evidence/M12/qemu/gdb_serial.log -s -S -no-reboot -no-shutdown -cdrom build/mcsos.iso &
gdb build/kernel.elf
target remote localhost:1234
break m12_sync_selftest
break mcs_spin_lock
break mcs_lockdep_before_acquire
continue
```

Hasil:

```text
Ketiga breakpoint berhasil dipasang pada alamat yang benar (diverifikasi lewat "info address"), tetapi setelah "continue", sesi berakhir lewat interupsi manual (Ctrl+C) dengan eksekusi berada di cpu_hlt() pada loop akhir kmain(), bukan pada salah satu breakpoint. Percobaan perekaman ulang lewat "script evidence/M12/gdb-session.log" juga menghasilkan file kosong (0 byte). Simbol dan alamat fungsi berhasil diverifikasi statis lewat "disassemble /m mcs_spin_lock" dan "nm build/kernel.elf | grep mcs_spin", tetapi bukti dinamis (breakpoint benar-benar tertangkap saat fungsi dipanggil) belum tersedia pada sesi ini.
```

Status: `[INCONCLUSIVE]`

### 12.5 Unit Test

```bash
make -f Makefile.m12 host-test
```

Hasil:

```text
./build/m12/m12_sync_host_test
[PASS] M12 synchronization host tests passed
```

Status: `[PASS]`

### 12.6 Stress/Fuzz/Fault Injection Test

Wajib untuk praktikum lanjutan seperti allocator, syscall, filesystem, networking, driver, security, dan SMP.

```text
Belum diterapkan pada M12. Host unit test yang ada baru mencakup jalur normal spinlock/mutex/lockdep; belum ada stress test pemanggilan lock secara konkuren dari banyak thread host (walau Makefile.m12 sudah menautkan -pthread), belum ada fuzzing terhadap urutan acquire/release acak untuk memicu violation_count pada lockdep, dan belum ada uji SMP nyata karena MCSOS masih single-core/kooperatif pada milestone ini.
```

Status: `[NA]`

### 12.7 Visual Evidence

| Screenshot | Lokasi file | Keterangan |
|---|---|---|
| `[screenshot]` | `[path]` | `[Tidak relevan pada M12, tidak ada output grafis; bukti berupa log serial teks dan transkrip terminal]` |

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| No. | Uji | Expected result | Actual result | Status | Evidence |
|---|---|---|---|---|---|
| 1 | `[Sintaks mcs_sync.h, lockdep.c, spinlock.c, mutex.c]` | `[Tidak ada error/warning]` | `[clang -Wall -Wextra -Werror bersih]` | `[PASS]` | `[evidence/M12/m12-build.log]` |
| 2 | `[Host unit test sinkronisasi]` | `[Seluruh skenario spinlock/mutex/lockdep lulus]` | `[PASS] M12 synchronization host tests passed]` | `[PASS]` | `[build/m12/host-test.log]` |
| 3 | `[Audit objek freestanding lockdep/spinlock/mutex]` | `[ELF64 x86_64, tidak ada undefined symbol]` | `[nm-undefined.txt kosong, readelf/objdump sesuai]` | `[PASS]` | `[evidence/M12/nm-undefined.txt, readelf-lockdep.txt, objdump-spinlock.txt]` |
| 4 | `[Integrasi selftest.c ke Makefile.m12]` | `[Build host-test dan freestanding sukses]` | `[Gagal 3 kali (source salah, missing separator, include path), berhasil setelah 3 perbaikan]` | `[PASS setelah perbaikan]` | `[evidence/M12/m12-build-after-selftest.log, m12-build-final.log]` |
| 5 | `[Build kernel penuh dengan modul sync M12]` | `[kernel.elf berhasil dibangun]` | `[Berhasil, 43K, seluruh assertion audit bawaan lulus]` | `[PASS]` | `[build/kernel.elf, build/kernel.syms.txt]` |
| 6 | `[Pembuatan ulang image ISO dengan Limine]` | `[build/mcsos.iso valid dan bootable]` | `[Berhasil, 4.2M]` | `[PASS]` | `[build/mcsos.iso]` |
| 7 | `[Boot QEMU dengan selftest sinkronisasi]` | `[Log memuat "sync selftest start"/"passed"]` | `[Log serial sesuai ekspektasi, muncul sebelum M9 scheduler]` | `[PASS]` | `[evidence/M12/qemu/m12_serial.log]` |
| 8 | `[Sesi debugging GDB dengan breakpoint fungsi sync]` | `[Breakpoint tertangkap saat fungsi dipanggil]` | `[Breakpoint terpasang tetapi tidak terbukti tertangkap; sesi berakhir di cpu_hlt lewat Ctrl+C]` | `[INCONCLUSIVE]` | `[transkrip sesi gdb, evidence/M12/gdb-session.log (0 byte)]` |
| 9 | `[Commit dan push repository]` | `[Perubahan tersimpan di Git dan remote]` | `[Commit 673ea92, branch terpush]` | `[PASS]` | `[git log, git push output]` |

### 13.2 Log Penting

```text
[PASS] M12 synchronization host tests passed

[M8] kernel heap initialized
[M12] sync selftest start
[M12] sync selftest passed
[M10] user region initialized
```

---

## 14. Analisis Teknis

```text
Pemisahan tegas antara objek "murni" (lockdep.c, spinlock.c, mutex.c — tidak bergantung header kernel) dan objek "terintegrasi" (selftest.c — bergantung mcsos/kernel/log.h dan mcsos/kernel/panic.h) terbukti penting sekaligus menjadi sumber tiga kegagalan build berurutan pada sesi ini: begitu selftest.c ikut dimasukkan ke daftar sumber host-test, header kernel yang hanya relevan untuk build freestanding langsung tidak ditemukan pada lingkungan host. Setelah SYNC_SRCS (host) dipisah dari KERNEL_SYNC_SRCS (freestanding, menambahkan selftest.c), masalah include path -Ikernel/include yang belum lengkap pada KERNEL_CFLAGS muncul sebagai lapisan kegagalan berikutnya — pola yang serupa dengan kegagalan -Iinclude pada laporan M10 sebelumnya, menunjukkan bahwa disiplin penambahan include path pada setiap Makefile baru tetap menjadi risiko berulang di proyek ini. Dari sisi desain sinkronisasi, penggunaan __atomic_exchange_n dengan ACQUIRE pada mcs_spin_try_lock dan __atomic_store_n dengan RELEASE pada mcs_spin_unlock sudah mengikuti pola acquire-release standar untuk spinlock, dan instruksi PAUSE pada busy-wait loop sesuai praktik x86_64. Sesi debugging GDB pada Langkah 12 menunjukkan keterbatasan penting: meskipun breakpoint berhasil dipasang pada alamat simbol yang benar, "continue" berakhir dengan program sudah berada pada loop hlt akhir kmain() saat diinterupsi manual, sehingga klaim "breakpoint berfungsi" tidak dapat dibuktikan pada sesi ini — sebuah pengingat bahwa keberhasilan memasang breakpoint (alamat benar, GDB menerima perintah) tidak sama dengan bukti breakpoint benar-benar tertangkap saat fungsi dieksekusi.
```

---

## 15. Debugging dan Failure Modes

### 15.1 `selftest.c` Salah Dimasukkan ke Daftar Sumber Host-Test

```text
Root cause: Pada revisi awal Makefile.m12, variabel SYNC_SRCS (dipakai baik untuk kompilasi HOSTCC pada target host-test maupun sebagai basis KERNEL_SYNC_SRCS pada target freestanding) sempat memuat kernel/sync/selftest.c secara langsung. Karena selftest.c meng-include <mcsos/kernel/log.h> (header khusus kernel yang secara fisik berada di kernel/include/mcsos/kernel/log.h dan tidak relevan untuk build host), kompilasi host gagal.
Diagnosis: "make -f Makefile.m12 all CC=clang" gagal dengan "kernel/sync/selftest.c:2:10: fatal error: mcsos/kernel/log.h: No such file or directory" tepat pada tahap host-test, sebelum tahap freestanding sempat dijalankan.
Perbaikan: memisahkan SYNC_SRCS (tanpa selftest.c, dipakai HOSTCC) dari KERNEL_SYNC_SRCS := $(SYNC_SRCS) kernel/sync/selftest.c (dipakai hanya pada target freestanding dengan $(CC) dan flag -Ikernel/include).
```

### 15.2 Kegagalan "missing separator" pada Makefile.m12

```text
Root cause: baris resep baru yang ditambahkan untuk mengompilasi selftest.c pada target freestanding tertulis dengan delapan spasi, bukan karakter tab (atau prefiks resep kustom) yang dikenali GNU Make.
Diagnosis: "make -f Makefile.m12 clean" dan "make -f Makefile.m12 all" keduanya gagal dengan "Makefile.m12:29: *** missing separator (did you mean TAB instead of 8 spaces?). Stop." — pesan Make secara eksplisit menunjukkan baris dan penyebabnya.
Perbaikan: membuka kembali Makefile.m12 lewat nano dan mengganti awalan baris ke-29 menjadi karakter tab yang benar, konsisten dengan seluruh baris resep lain pada file yang sama.
```

### 15.3 Header `mcsos/kernel/log.h` Tidak Ditemukan pada Kompilasi Freestanding `selftest.c`

```text
Root cause: KERNEL_CFLAGS pada Makefile.m12 hanya memuat -Iinclude, belum memuat -Ikernel/include, padahal mcsos/kernel/log.h dan mcsos/kernel/panic.h secara fisik berada di bawah kernel/include/.
Diagnosis: setelah dua perbaikan sebelumnya, kompilasi freestanding lockdep.c/spinlock.c/mutex.c berhasil, tetapi selftest.c tetap gagal dengan "fatal error: 'mcsos/kernel/log.h' file not found" karena flag include belum lengkap.
Perbaikan: menambahkan " -Ikernel/include" pada KERNEL_CFLAGS di Makefile.m12, setelah itu seluruh objek freestanding (termasuk selftest.o) berhasil dikompilasi.
```

### 15.4 Assertion Audit Gagal karena Simbol Undefined pada `selftest.o`

```text
Root cause: setelah selftest.o berhasil dikompilasi, target audit pada Makefile.m12 semula menjalankan "nm -u" terhadap keempat objek (lockdep.o, spinlock.o, mutex.o, selftest.o) dan memeriksa assertion "@! grep -q ' U ' nm-undefined.txt". Karena selftest.o secara sah masih memiliki referensi undefined (kernel_panic_at, log_writeln, dan seluruh fungsi mcs_spin_*/mcs_mutex_*/mcs_lockdep_init yang baru diresolusi saat ditautkan ke build/kernel.elf), assertion ini gagal walau tidak ada kesalahan kode yang sesungguhnya.
Diagnosis: "make -f Makefile.m12 all" berhenti dengan "make: *** [Makefile.m12:36: audit] Error 1" tepat setelah nm -u menampilkan daftar simbol undefined milik selftest.o.
Perbaikan: mengubah target audit agar "nm -u" dan assertion "grep -q ' U '" hanya diterapkan pada lockdep.o, spinlock.o, dan mutex.o (objek yang memang dirancang berdiri sendiri tanpa dependensi eksternal), sementara selftest.o diverifikasi kebenarannya lewat build kernel penuh (Langkah 10) yang membuktikan seluruh simbol tersebut benar-benar tertaut.
```

### 15.5 Panic Path

```text
Panic path M12 memakai makro KERNEL_PANIC yang sudah tersedia sejak milestone sebelumnya (kernel_panic_at, dipanggil lewat KERNEL_PANIC(reason, code)). Pada M12, panic dipicu dari dalam m12_sync_selftest() bila validasi spinlock, mutex, atau lockdep tidak sesuai ekspektasi. Pada sesi ini panic tersebut tidak pernah terpicu karena log serial QEMU menunjukkan "[M12] sync selftest passed" tanpa diikuti pesan panic apa pun, sehingga jalur panic M12 belum teruji secara aktual pada runtime, hanya tersedia sebagai mekanisme defensif — sama seperti pola yang tercatat pada laporan M10 sebelumnya untuk panic path syscall.
```

### 15.6 Sesi Debugging GDB Tidak Menangkap Breakpoint Secara Meyakinkan

```text
Root cause: belum dapat dipastikan secara pasti pada sesi ini apakah breakpoint memang tidak pernah tercapai (mis. karena m12_sync_selftest telah selesai dieksekusi sebelum sambungan GDB benar-benar aktif) atau tercapai namun perintah "continue" berikutnya sudah dijalankan lagi tanpa sempat diperiksa keadaannya sebelum sesi berakhir lewat Ctrl+C.
Diagnosis: transkrip sesi menunjukkan tiga breakpoint berhasil dipasang pada alamat yang benar, tetapi setelah "continue", backtrace pertama yang benar-benar diperiksa (bt) menunjukkan program sudah berada pada cpu_hlt() di dalam loop tak berhingga kmain() (setelah tahap "sti: enabling interrupts"), yang berarti seluruh urutan boot termasuk m12_sync_selftest() sudah selesai dieksekusi.
Perbaikan yang direncanakan: pada sesi berikutnya, menjalankan GDB dengan skrip perintah non-interaktif (mis. "gdb -x script.gdb") yang otomatis berhenti pada breakpoint pertama dan mencetak bukti (bt, info registers, print variabel lokal) sebelum melanjutkan, serta merekam seluruh sesi dengan "script" sejak awal (bukan setelah sesi berjalan) agar bukti breakpoint benar-benar tertangkap tidak hilang.
```

### 15.7 Ringkasan Failure Mode

| Kegagalan | Gejala | Root cause | Perbaikan | Status |
|---|---|---|---|---|
| `[selftest.c masuk daftar host-test]` | `[fatal error: mcsos/kernel/log.h not found saat host-test]` | `[SYNC_SRCS belum dipisah dari KERNEL_SYNC_SRCS]` | `[Memisahkan SYNC_SRCS dan KERNEL_SYNC_SRCS]` | `[Selesai]` |
| `[Makefile.m12 missing separator]` | `[make clean/all gagal total]` | `[Baris resep baru memakai 8 spasi, bukan tab]` | `[Mengganti awalan baris dengan tab]` | `[Selesai]` |
| `[Header log.h tidak ditemukan saat freestanding]` | `[fatal error saat kompilasi selftest.c]` | `[KERNEL_CFLAGS belum memuat -Ikernel/include]` | `[Menambahkan -Ikernel/include]` | `[Selesai]` |
| `[Assertion audit gagal karena selftest.o]` | `[make audit Error 1]` | `[nm -u diterapkan pada selftest.o yang secara sah punya undefined symbol]` | `[Membatasi audit nm -u hanya pada lockdep.o/spinlock.o/mutex.o]` | `[Selesai]` |
| `[Breakpoint GDB tidak terbukti tertangkap]` | `[continue berakhir di cpu_hlt, bukan pada breakpoint]` | `[Belum dipastikan — kemungkinan timing sambungan GDB atau kesalahan prosedur]` | `[Direncanakan memakai skrip GDB non-interaktif pada sesi berikutnya]` | `[Belum selesai]` |

---

## 16. Prosedur Rollback

```text
Repository tidak memerlukan rollback berat pada sesi M12 ini, karena seluruh kesalahan yang terjadi (tiga kelas kegagalan Makefile.m12, sesi GDB yang inconclusive) bersifat dapat diperbaiki di tempat (in place) tanpa merusak file sumber lockdep.c/spinlock.c/mutex.c/selftest.c yang sudah benar. Prosedur rollback yang tersedia dan disiapkan sebagai jaring pengaman:
1. "git status --short" dijalankan sebelum setiap commit untuk memastikan hanya perubahan yang dimaksud yang disertakan (dipraktikkan pada Langkah 13).
2. Bila commit 673ea92 perlu dibatalkan, "git revert 673ea92" atau "git reset --hard a701faa" (commit akhir M11) dapat dipakai untuk kembali ke kondisi sebelum M12, karena branch praktikum/m12-sync terisolasi dari branch praktikum-m11-elf-user-loader.
3. File sampah peninggalan sesi sebelumnya (M11.txt, m11_host_test, ~$M11.txt) telah dibersihkan lebih dulu pada Langkah 1 sebagai bagian dari disiplin kebersihan repository sebelum branch baru dibuka.
Prosedur "git reset --hard" maupun "git revert" belum benar-benar dieksekusi pada sesi M12 ini karena tidak diperlukan; keduanya dicatat sebagai prosedur yang tersedia, bukan yang telah diuji pada milestone ini.
```

---

## 17. Keamanan dan Reliability

### 17.1 Ringkasan Keamanan

```text
Keamanan utama M12 terletak pada disiplin ownership mutex (mcs_mutex_unlock menolak pembukaan oleh owner_id yang tidak sesuai dengan MCS_SYNC_EPERM) dan pembatasan kedalaman pencatatan lockdep (MCS_LOCKDEP_MAX_HELD=16, ditolak dengan MCS_SYNC_EOVERFLOW bila terlampaui) sehingga struktur data pembukuan lock tidak dapat ditulis di luar batas array held_class/held_name. Namun demikian, mcs_spin_lock belum memiliki mekanisme timeout sehingga secara desain rentan terhadap busy-wait tanpa akhir bila pemanggil lain lupa memanggil mcs_spin_unlock, dan lockdep pada sesi ini baru mencatat/mendeteksi pelanggaran urutan (violation_count) tanpa mengambil tindakan pemulihan otomatis (mis. menolak acquire berikutnya atau memicu panic).
```

### 17.2 Tabel Ancaman

| Ancaman | Vektor | Mitigasi saat ini | Kecukupan |
|---|---|---|---|
| `[Pembukaan mutex oleh pihak yang bukan pemiliknya]` | `[owner_id yang salah pada mcs_mutex_unlock]` | `[Perbandingan owner_id dengan owner tercatat, return MCS_SYNC_EPERM bila tidak sesuai]` | `[Cukup untuk kasus yang diuji host test]` |
| `[Overflow pencatatan lockdep]` | `[Pemanggilan mcs_lockdep_before_acquire melebihi MCS_LOCKDEP_MAX_HELD kali tanpa release]` | `[Pengecekan depth dan pengembalian MCS_SYNC_EOVERFLOW]` | `[Cukup secara desain; belum ada host test khusus yang memicu skenario overflow ini]` |
| `[Busy-wait tanpa akhir pada spinlock]` | `[Pemanggil lupa memanggil mcs_spin_unlock]` | `[Belum ada — tidak ada timeout atau watchdog]` | `[Belum cukup, dicatat sebagai known issue]` |
| `[Konkurensi pemanggilan sinkronisasi dari banyak thread/CPU nyata]` | `[Struktur global bila dipakai lintas CPU pada SMP]` | `[Belum relevan karena MCSOS masih single-core/kooperatif]` | `[Belum diuji untuk skenario SMP]` |

### 17.3 Reliability

```text
Reliability level host (unit test) dan level build (audit ELF, assertion Makefile.m12) sudah baik: target host-test dan audit lulus secara konsisten dan dapat diulang dari clean checkout ("make -f Makefile.m12 clean" lalu target terkait). Reliability level runtime juga terbukti lewat log serial QEMU nyata yang menunjukkan m12_sync_selftest() berjalan dan lulus pada setiap boot. Yang belum terbukti reliable adalah verifikasi dinamis lewat GDB (breakpoint belum terbukti tertangkap) dan perilaku sistem saat primitif sinkronisasi dipanggil bersamaan dari lebih dari satu thread/CPU setelah preemption atau SMP diimplementasikan pada milestone lanjutan.
```

---

## 18. Pembagian Kerja Kelompok

Isi bagian ini hanya jika praktikum dikerjakan berkelompok. Untuk pengerjaan individu, tulis "Tidak berlaku".

```text
Tidak berlaku. Praktikum M12 dikerjakan secara individu oleh Syifa Nurzimah (NIM 25832074009).
```

### 18.1 Mekanisme Koordinasi

```text
Tidak berlaku untuk pengerjaan individu.
```

### 18.2 Evaluasi Kontribusi

| Anggota | Persentase kontribusi yang disepakati | Bukti | Catatan |
|---|---:|---|---|
| `[Syifa Nurzimah]` | `[100%]` | `[commit 673ea92]` | `[Pengerjaan individu]` |

---

## 19. Kriteria Lulus Praktikum

Bagian ini wajib diisi. Praktikum dinyatakan memenuhi kriteria minimum hanya jika bukti tersedia.

| Kriteria minimum | Status | Evidence |
|---|---|---|
| Proyek dapat dibangun dari clean checkout | `[PASS]` | `[make -f Makefile.m12 clean && make -f Makefile.m12 all, make clean && make all]` |
| Perintah build terdokumentasi | `[PASS]` | `[Bagian 10 Langkah Kerja Implementasi]` |
| QEMU boot atau test target berjalan deterministik | `[PASS]` | `[evidence/M12/qemu/m12_serial.log memuat selftest sinkronisasi]` |
| Semua unit test/praktikum test relevan lulus | `[PASS]` | `[[PASS] M12 synchronization host tests passed]` |
| Log serial disimpan | `[PASS]` | `[evidence/M12/m12-qemu-serial-final.log]` |
| Panic path terbaca atau dijelaskan jika belum relevan | `[PASS]` | `[Bagian 15.5 Panic Path]` |
| Tidak ada warning kritis pada build | `[PASS]` | `[-Wall -Wextra -Werror bersih pada seluruh langkah setelah perbaikan]` |
| Perubahan Git terkomit | `[PASS]` | `[commit 673ea92]` |
| Desain dan failure mode dijelaskan | `[PASS]` | `[Bagian 9 Desain Teknis dan 15 Failure Modes]` |
| Laporan berisi screenshot/log yang cukup | `[PASS]` | `[Lampiran evidence terminal]` |

Kriteria tambahan untuk praktikum lanjutan:

| Kriteria lanjutan | Status | Evidence |
|---|---|---|
| Static analysis dijalankan | `[PASS]` | `[clang -Wall -Wextra -Werror pada seluruh file M12]` |
| Stress test dijalankan | `[NA]` | `[belum diterapkan pada M12]` |
| Fuzzing atau malformed-input test dijalankan | `[NA]` | `[belum diterapkan pada M12]` |
| Fault injection dijalankan | `[NA]` | `[belum diterapkan pada M12]` |
| Disassembly/readelf evidence tersedia | `[PASS]` | `[evidence/M12/objdump-spinlock.txt, readelf-lockdep.txt, build/kernel.disasm.txt]` |
| Review keamanan dilakukan | `[PASS]` | `[Bagian 17 Keamanan dan Reliability]` |
| Rollback diuji | `[NA]` | `[Prosedur tersedia (Bagian 16) tetapi belum benar-benar dieksekusi pada M12]` |
| GDB/debugger dipakai dan bukti tertangkap | `[NA]` | `[Breakpoint dipasang tetapi belum terbukti tertangkap, lihat Bagian 12.4]` |

---

## 20. Readiness Review

Pilih satu status dengan alasan berbasis bukti.

| Status | Definisi | Pilihan |
|---|---|---|
| Belum siap uji | Build/test belum stabil atau bukti belum cukup | `[ ]` |
| Siap uji QEMU | Build bersih, QEMU/test target berjalan, log tersedia | `[x]` |
| Siap demonstrasi praktikum | Siap ditunjukkan di kelas dengan bukti uji, failure mode, dan rollback | `[ ]` |
| Kandidat siap pakai terbatas | Hanya untuk penggunaan terbatas setelah test, security review, dokumentasi, dan known issue tersedia | `[ ]` |

Alasan readiness:

```text
Seluruh tahapan build dan test level host untuk M12 berhasil dijalankan: sintaks mcs_sync.h, lockdep.c, spinlock.c, dan mutex.c valid, host unit test menunjukkan "[PASS] M12 synchronization host tests passed", audit objek freestanding menunjukkan struktur ELF64 x86_64 yang benar tanpa simbol undefined pada objek inti, kernel penuh berhasil dibangun setelah tiga kelas kegagalan Makefile.m12 diperbaiki, image ISO berhasil dibuat ulang, dan boot QEMU nyata berhasil dijalankan dengan log serial yang membuktikan "[M12] sync selftest start"/"passed" berjalan sebelum scheduler M9 diinisialisasi. Namun demikian, sesi debugging GDB (breakpoint pada m12_sync_selftest, mcs_spin_lock, dan mcs_lockdep_before_acquire) belum membuktikan breakpoint benar-benar tertangkap, sehingga status readiness ditetapkan "Siap uji QEMU" (build dan smoke test log serial sudah terbukti kuat) dan bukan "Siap demonstrasi praktikum" (yang mensyaratkan bukti debugging dinamis yang meyakinkan).
```

Known issues:

| No. | Issue | Dampak | Workaround | Target perbaikan |
|---|---|---|---|---|
| 1 | `[Breakpoint GDB pada fungsi sinkronisasi belum terbukti tertangkap]` | `[Verifikasi dinamis jalur eksekusi spinlock/mutex/lockdep di kernel nyata belum lengkap]` | `[Verifikasi lewat log serial QEMU dan disassembly statis nm/objdump]` | `[Memakai skrip GDB non-interaktif (gdb -x) yang otomatis stop di breakpoint pertama dan mencetak bukti sebelum sesi berikutnya berjalan]` |
| 2 | `[mcs_spin_lock belum memiliki timeout]` | `[Berpotensi busy-wait tanpa akhir bila pemanggil lupa unlock]` | `[Disiplin pemanggilan lock/unlock berpasangan pada m12_sync_selftest]` | `[Menambahkan mekanisme timeout/deadline pada spinlock di milestone lanjutan]` |
| 3 | `[Lockdep baru mencatat, belum mengambil tindakan pemulihan atas pelanggaran urutan]` | `[violation_count bertambah tetapi acquire berikutnya tetap diizinkan]` | `[Pemantauan manual terhadap violation_count]` | `[Menambahkan kebijakan penolakan/panic otomatis saat violation_count meningkat, pada milestone security (M13/lanjutan)]` |
| 4 | `[Belum ada stress/fuzz test untuk kombinasi acquire/release acak]` | `[Klaim robustness lockdep terhadap pola locking tak terduga belum sepenuhnya diverifikasi]` | `[Validasi manual pada skenario normal host test]` | `[Menambahkan fuzz test pada milestone lanjutan]` |
| 5 | `[evidence/M12/gdb-session.log tersimpan kosong (0 byte)]` | `[Tidak ada rekaman formal sesi debugging sebagai bukti terpisah]` | `[Transkrip terminal manual dijadikan bukti pada laporan ini]` | `[Memulai perekaman "script" sebelum sesi GDB dimulai, bukan setelahnya]` |
| 6 | `[Belum ada locking pada level SMP/multi-CPU nyata]` | `[Primitif sinkronisasi belum diuji pada skenario benar-benar konkuren]` | `[MCSOS masih single-core/kooperatif sehingga risiko saat ini rendah]` | `[Menambahkan pengujian SMP saat milestone M13 (SMP/scalability) dikerjakan]` |

Keputusan akhir:

```text
Berdasarkan hasil make -f Makefile.m12 host-test, make -f Makefile.m12 audit, build kernel penuh, dan boot QEMU dengan log serial yang seluruhnya berhasil dijalankan, praktikum M12 dinyatakan siap uji QEMU, dengan catatan bahwa verifikasi dinamis lewat GDB perlu diulang dengan prosedur yang lebih ketat pada kesempatan berikutnya sebelum status dinaikkan menjadi siap demonstrasi.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Indikator nilai penuh | Nilai |
|---|---:|---|---:|
| Kebenaran fungsional | 30 | Implementasi memenuhi target praktikum, build/test lulus, output sesuai expected result | `[27]` |
| Kualitas desain dan invariants | 20 | Desain jelas, kontrak antarmuka eksplisit, invariants/ownership/locking terdokumentasi | `[17]` |
| Pengujian dan bukti | 20 | Unit/integration/QEMU/static/fuzz/stress evidence memadai sesuai tingkat praktikum | `[14]` |
| Debugging dan failure analysis | 10 | Failure mode, triage, panic/log, dan rollback dianalisis | `[9]` |
| Keamanan dan robustness | 10 | Boundary, input validation, privilege, memory safety, dan negative tests dibahas | `[7]` |
| Dokumentasi dan laporan | 10 | Laporan rapi, lengkap, dapat direproduksi, memakai referensi yang layak | `[9]` |
| **Total** | **100** |  | `[83]` |

Catatan penilai:

```text
[Diisi dosen/asisten.]
```

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Kontrak primitif sinkronisasi MCSOS (mcs_spinlock_t, mcs_mutex_t, mcs_lockdep_state_t, kode status MCS_SYNC_*) berhasil didesain dan diimplementasikan dalam C17 freestanding pada include/mcs_sync.h, kernel/sync/lockdep.c, kernel/sync/spinlock.c, dan kernel/sync/mutex.c, lengkap dengan spinlock atomic berbasis xchg dan pause, mutex ber-owner, dan lockdep pencatat urutan lock. Host unit test lulus untuk seluruh skenario, dan objek freestanding lockdep/spinlock/mutex teraudit tanpa simbol undefined lewat nm/readelf/objdump. Setelah memperbaiki tiga kelas kegagalan Makefile.m12 (selftest.c salah masuk daftar host-test, missing separator, dan include path -Ikernel/include yang belum lengkap), kernel penuh berhasil dibangun dengan modul sinkronisasi terintegrasi lewat kernel/sync/selftest.c dan pemanggilan m12_sync_selftest() pada kmain(), image ISO berhasil dibuat ulang memakai Limine, dan boot QEMU nyata berhasil dijalankan dengan log serial yang membuktikan "[M12] sync selftest start" dan "[M12] sync selftest passed" berjalan tepat setelah kernel heap M8 siap dan sebelum scheduler M9 diinisialisasi. Seluruh perubahan telah dikomit (673ea92) dan dipush ke repository pada branch praktikum/m12-sync.
```

### 22.2 Yang Belum Berhasil

```text
Sesi debugging remote GDB dengan breakpoint pada m12_sync_selftest, mcs_spin_lock, dan mcs_lockdep_before_acquire belum membuktikan breakpoint benar-benar tertangkap saat fungsi dipanggil; sesi berakhir dengan program sudah berada pada loop hlt akhir kmain() ketika diinterupsi manual, dan percobaan perekaman ulang lewat "script" menghasilkan file kosong. Mcs_spin_lock belum memiliki mekanisme timeout sehingga secara desain rentan busy-wait tanpa akhir, lockdep baru mencatat pelanggaran urutan (violation_count) tanpa mengambil tindakan pemulihan otomatis, dan belum ada stress/fuzz test untuk kombinasi acquire/release acak maupun pengujian pada skenario SMP nyata karena MCSOS masih single-core/kooperatif pada milestone ini.
```

### 22.3 Rencana Perbaikan

```text
Mengulang sesi debugging GDB memakai skrip non-interaktif (gdb -x script.gdb) yang otomatis berhenti pada breakpoint pertama dan mencetak bukti (backtrace, register, variabel lokal) sebelum melanjutkan, serta memulai perekaman "script" sejak sebelum sesi dimulai. Menambahkan mekanisme timeout/deadline pada mcs_spin_lock, menambahkan kebijakan penolakan atau panic otomatis pada lockdep saat violation_count meningkat, menambahkan stress test dan fuzz test untuk kombinasi acquire/release acak pada spinlock/mutex/lockdep, serta menyiapkan pengujian pada skenario SMP nyata saat milestone M13 (SMP, scalability, lock stress) dikerjakan pada semester berikutnya.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
673ea92 Complete M12 synchronization primitives and lockdep
a701faa M11: Complete M11 ELF user loader integration
889411f M10: implement syscall ABI and INT80 interface
4453b81 M9: implement cooperative kernel scheduler
a44e07b checkpoint before M9 scheduler
03657c2 M8 kernel heap implementation
```

### Lampiran B — Diff Ringkas

```diff
Perubahan utama:
- Menambahkan include/mcs_sync.h (kontrak primitif sinkronisasi, 1799 byte)
- Menambahkan kernel/sync/lockdep.c (lock-dependency validator, 2041 byte)
- Menambahkan kernel/sync/spinlock.c (spinlock atomic, 1178 byte)
- Menambahkan kernel/sync/mutex.c (mutex ber-owner, 1620 byte)
- Menambahkan kernel/sync/selftest.c (selftest kernel nyata, 53 baris) dan include/mcs_sync_selftest.h
- Menambahkan tests/m12_sync_host_test.c (host unit test sinkronisasi, 3264 byte)
- Menambahkan Makefile.m12 (target host-test, freestanding, audit, clean, 1371 byte), diperbaiki 3 kali selama sesi
- Mengubah kernel/core/kmain.c: menyisipkan pemanggilan m12_sync_selftest() setelah m8_heap_bootstrap()
- Membuat ulang build/mcsos.iso lewat tools/scripts/make_iso.sh (Limine + xorriso)
```

### Lampiran C — Log Build Lengkap

```text
[PASS] M12 synchronization host tests passed
evidence/M12/nm-undefined.txt kosong untuk lockdep.o/spinlock.o/mutex.o (tidak ada simbol undefined)
readelf -h build/m12/lockdep.o: ELF64, Machine Advanced Micro Devices X86-64, Type REL
build/kernel.elf berhasil dibangun dengan seluruh assertion audit bawaan (ELF64, Machine X86-64, kmain, x86_64_idt_init, x86_64_trap_dispatch, iretq, lidt) lulus
```

### Lampiran D — Log QEMU Lengkap

```text
limine: Loading executable `boot():/boot/kernel.elf`...
[MCSOS:M5] boot: external interrupt bring-up start
idt_base=0xffffffff80008000
idt_limit=0x0000000000000fff
[M4] IDT loaded
[MCSOS:M5] idt: loaded
[MCSOS:M5] pic: remapped; mask master=0x00000000000000fe slave=0x00000000000000ff
[MCSOS:M5] pit: configured 100Hz
[M8] kernel heap initialized
[M12] sync selftest start
[M12] sync selftest passed
[M10] user region initialized
[M10] syscall subsystem initialized
[M10] syscall ping ok
[M10] syscall get_ticks ok
[M10] syscall smoke done
[M11] before call
[M11] smoke entered
[M11] elf: plan ok
[M11] entry=0x0000000000401000 segments=0x0000000000000001
[M11] user image plan ready
[M11] after call
[M9] scheduler initialized
[MCSOS:M5] sti: enabling interrupts
[MCSOS:TIMER] ticks=0x0000000000000064
[MCSOS:TIMER] ticks=0x00000000000000c8
[MCSOS:TIMER] ticks=0x000000000000012c
[MCSOS:TIMER] ticks=0x0000000000000190
[MCSOS:TIMER] ticks=0x00000000000001f4
```

### Lampiran E — Output Readelf/Objdump

```text
readelf -h build/m12/lockdep.o : ELF64, Type REL, Machine Advanced Micro Devices X86-64
objdump -d build/m12/spinlock.o : mcs_spin_init, mcs_spin_try_lock (xchg), mcs_spin_lock (pause), mcs_spin_unlock, mcs_spin_is_locked
nm -u build/m12/lockdep.o build/m12/spinlock.o build/m12/mutex.o : kosong (tidak ada simbol undefined)
sha256sum build/mcsos.iso : e10dd97a998b3fc95f6bec725d4ead1b4d0b40b71593f6f2d6a375add6d84e74
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[path/screenshot]` | `[belum dilampirkan pada transkrip yang tersedia]` |

### Lampiran G — Bukti Tambahan

```text
build/m12/m12_sync_host_test
build/m12/lockdep.o
build/m12/spinlock.o
build/m12/mutex.o
build/m12/selftest.o
build/m12/nm-undefined.txt
build/m12/readelf-lockdep.txt
build/m12/objdump-spinlock.txt
build/m12/sha256sums.txt
build/kernel.elf
build/kernel.syms.txt
build/kernel.disasm.txt
build/kernel.map
build/mcsos.iso
evidence/M12/preflight.log
evidence/M12/m12-build.log
evidence/M12/m12-build-after-selftest.log
evidence/M12/m12-build-final.log
evidence/M12/nm-undefined.txt
evidence/M12/readelf-lockdep.txt
evidence/M12/objdump-spinlock.txt
evidence/M12/sha256sums.txt
evidence/M12/m12-sha256-final.txt
evidence/M12/m12-qemu-serial-final.log
evidence/M12/gdb-session.log
evidence/M12/qemu/m12_serial.log
```

---

## 24. Daftar Referensi

Gunakan format IEEE. Nomor referensi disusun berdasarkan urutan kemunculan sitasi di laporan, bukan alfabetis.

Referensi yang benar-benar dipakai dalam laporan:

```text
[1] R. H. Arpaci-Dusseau and A. C. Arpaci-Dusseau, Operating Systems: Three Easy Pieces, bab Concurrency: Locks dan Condition Variables. [Online]. Available: [URL]. Accessed: [tanggal akses].
[2] Intel Corporation, Intel 64 and IA-32 Architectures Software Developer's Manual, bagian Locked Atomic Operations (XCHG) dan PAUSE Instruction. [Online]. Available: [URL]. Accessed: [tanggal akses].
[3] GNU Binutils Documentation (nm, readelf, objdump). [Online]. Available: [URL]. Accessed: [tanggal akses].
[4] GNU GDB Documentation (Remote Debugging, target remote, breakpoint). [Online]. Available: [URL]. Accessed: [tanggal akses].
[5] Limine Bootloader Documentation dan xorriso (RockRidge/ISO9660) Documentation. [Online]. Available: [URL]. Accessed: [tanggal akses].
[6] Git Documentation (git-commit, git-push, git-branch). [Online]. Available: [URL]. Accessed: [tanggal akses].
```

---

## 25. Checklist Final Sebelum Pengumpulan

| Checklist | Status |
|---|---|
| Semua placeholder `[isi ...]` sudah diganti | `[Tidak — tanggal pengumpulan dan catatan penilai masih perlu diisi mahasiswa/dosen]` |
| Metadata laporan lengkap | `[Ya]` |
| Commit awal dan akhir dicatat | `[Ya — commit awal a701faa, commit akhir 673ea92]` |
| Perintah build dan test dapat dijalankan ulang | `[Ya]` |
| Log build dilampirkan | `[Ya]` |
| Log QEMU/test dilampirkan | `[Ya]` |
| Artefak penting diberi hash | `[Ya — evidence/M12/sha256sums.txt, evidence/M12/m12-sha256-final.txt]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya — status GDB dilaporkan sebagai inconclusive, bukan diklaim sukses]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya, sebagai penilaian mandiri]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya/kami mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
673ea92
```

Status akhir yang diklaim:

```text
Siap uji QEMU
```

Ringkasan satu paragraf:

```text
Praktikum M12 berhasil menghasilkan primitif sinkronisasi kernel MCSOS (spinlock, mutex, dan lock-dependency validator) lengkap dengan kontrak header, host unit test yang lulus, dan audit objek freestanding tanpa simbol undefined. Setelah memperbaiki tiga kelas kegagalan Makefile.m12 (selftest.c yang salah masuk daftar host-test, kesalahan missing separator, dan include path -Ikernel/include yang belum lengkap), kernel penuh berhasil dibangun dengan selftest sinkronisasi terintegrasi ke kmain(), image ISO berhasil dibuat ulang memakai Limine, dan boot QEMU nyata berhasil dijalankan dengan log serial yang membuktikan "[M12] sync selftest start" dan "[M12] sync selftest passed" berjalan sebelum scheduler M9 diinisialisasi. Keterbatasan utama adalah sesi debugging remote GDB pada tiga breakpoint fungsi sinkronisasi belum membuktikan breakpoint benar-benar tertangkap (sesi berakhir pada loop hlt akhir kmain() setelah interupsi manual), mcs_spin_lock belum memiliki timeout, lockdep baru mencatat pelanggaran urutan tanpa tindakan pemulihan otomatis, dan belum ada stress/fuzz test untuk kombinasi acquire/release acak maupun pengujian SMP nyata. Langkah berikutnya adalah mengulang sesi GDB dengan skrip non-interaktif yang lebih ketat, menambahkan timeout pada spinlock, menambahkan kebijakan otomatis pada lockdep saat pelanggaran terdeteksi, dan menambahkan pengujian keamanan serta konkurensi yang lebih menyeluruh pada milestone berikutnya (M13 SMP/scalability).
```
