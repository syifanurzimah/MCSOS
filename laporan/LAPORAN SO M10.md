 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M10

**Nama file laporan:** `laporan_praktikum_m10_25832074009.md`
**Nama sistem operasi:** MCSOS versi 260502
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.
**Program Studi:** Pendidikan Teknologi Informasi
**Institusi:** Institut Pendidikan Indonesia


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M10]` |
| Judul praktikum | `[Syscall ABI dan Antarmuka INT 0x80: Kontrak Frame Syscall, Dispatch Table, Validasi Pointer User, dan Stub Assembly x86_64]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[isi jika kelompok]` |
| Anggota kelompok | `[tidak berlaku]` |
| Tanggal praktikum | `[2026-07-06 s.d. 2026-07-07]` |
| Tanggal pengumpulan | `[YYYY-MM-DD]` |
| Repository | `[/home/syifa/src/mcsos]` |
| Branch | `[praktikum/m10-syscall-abi]` |
| Commit awal | `` `[4453b81 — M9: implement cooperative kernel scheduler]` `` |
| Commit akhir | `` `[889411f — M10: implement syscall ABI and INT80 interface]` `` |
| Status readiness yang diklaim | `[Siap uji QEMU]` |

---

## 1. Sampul

# Laporan Praktikum `m10`
## `Syscall ABI dan Antarmuka INT 0x80: Kontrak Frame Syscall, Dispatch Table, Validasi Pointer User, dan Stub Assembly x86_64`

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

Saya menyatakan bahwa laporan ini disusun berdasarkan hasil praktikum yang saya kerjakan sendiri menggunakan lingkungan pengembangan WSL 2 Ubuntu sesuai panduan praktikum M10. Seluruh hasil yang dilaporkan didukung oleh bukti berupa command output, log, artefak build, dan commit repository yang terdokumentasi, termasuk proses debugging Makefile dan pembuatan image ISO yang benar-benar terjadi selama sesi.

| Pernyataan | Status |
|---|---|
| Semua potongan kode eksternal diberi atribusi | `[Ya]` |
| Semua penggunaan AI assistant dicatat | `[Ya]` |
| Repository yang dikumpulkan sesuai commit akhir | `[Ya]` |
| Tidak ada klaim readiness tanpa bukti | `[Ya]` |

Catatan penggunaan bantuan eksternal:

```text
Menggunakan dokumentasi resmi Clang/LLVM, GNU Make, GNU Binutils (nm, readelf, objdump), GNU Assembler/AT&T syntax x86_64, xorriso, Limine bootloader, QEMU, dan Git sebagai referensi teknis. Menggunakan AI Assistant untuk membantu memahami instruksi praktikum M10, melakukan debugging Makefile (kesalahan "missing separator" berulang akibat .RECIPEPREFIX kustom (karakter ">") yang hilang/tidak konsisten saat menambahkan target M10 lewat heredoc, serta kesalahan include path -Iinclude yang belum ada pada COMMON_CFLAGS/COMMON_ASFLAGS sehingga header mcsos/syscall.h tidak ditemukan saat build kernel penuh), menjelaskan langkah pembuatan image ISO dengan Limine dan xorriso, serta membantu penyusunan laporan praktikum. Seluruh command, konfigurasi, build, pengujian, pembuatan ISO, dan verifikasi log QEMU dijalankan serta diperiksa ulang secara mandiri pada lingkungan WSL 2 Ubuntu. Sesi dimulai pada branch m9-kernel-thread-scheduler (commit akhir 4453b81), kemudian dibuka branch baru praktikum/m10-syscall-abi untuk pekerjaan M10. Commit akhir repository: 889411f ("M10: implement syscall ABI and INT80 interface"), telah dipush ke remote origin/praktikum/m10-syscall-abi.
```

---

## 3. Tujuan Praktikum

Tuliskan tujuan teknis dan konseptual praktikum. Tujuan harus dapat diuji.

1. `Mendesain kontrak ABI syscall MCSOS (mcsos_syscall_frame_t, mcsos_syscall_nr_t, mcsos_syscall_status_t, mcsos_user_region_t, mcsos_syscall_ops_t) dalam header include/mcsos/syscall.h sebagai antarmuka tetap antara mode kernel dan mode pengguna.`
2. `Mengimplementasikan dispatcher syscall (mcsos_syscall_dispatch dan mcsos_syscall_dispatch_frame) berbasis tabel fungsi (g_table) yang menangani lima nomor syscall: MCSOS_SYS_PING, MCSOS_SYS_GET_TICKS, MCSOS_SYS_WRITE_SERIAL, MCSOS_SYS_YIELD, dan MCSOS_SYS_EXIT_THREAD.`
3. `Mengimplementasikan validasi pointer milik ruang pengguna (mcsos_user_check_range dan mcsos_copy_from_user) agar syscall yang menerima buffer dari user (mis. MCSOS_SYS_WRITE_SERIAL) tidak mengakses memori di luar region pengguna yang diizinkan.`
4. `Menulis stub assembly x86_64 (x86_64_syscall_int80_stub) untuk vektor interrupt 0x80 yang memindahkan register rax/rdi/rsi/rdx/r10/r8/r9 ke dalam mcsos_syscall_frame_t, memanggil mcsos_syscall_dispatch_frame, mengembalikan hasil ke rax, lalu iretq.`
5. `Menyusun host unit test (tests/test_syscall_host.c) yang menguji seluruh nomor syscall, copy_from_user pada rentang valid dan tidak valid, serta perilaku MCSOS_ENOSYS untuk nomor syscall yang tidak dikenal, sebelum objek freestanding dibangun.`
6. `Mengaudit objek freestanding hasil kompilasi syscall.c dan syscall_entry.S menggunakan nm, readelf, dan objdump untuk membuktikan struktur ELF64 x86_64, keberadaan simbol x86_64_syscall_int80_stub dan instruksi iretq, serta tidak adanya simbol undefined.`
7. `Mengintegrasikan subsistem syscall ke dalam kmain() MCSOS setelah kernel heap (M8) dan scheduler (M9) siap, membangun kernel penuh (kernel.elf), membuat image bootable (mcsos.iso) dengan Limine, dan memverifikasi log serial hasil boot QEMU menunjukkan pesan smoke test syscall.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Menjelaskan konsep ABI syscall, transisi mode pengguna ke mode kernel lewat software interrupt (INT 0x80), dan alasan kernel wajib memvalidasi pointer yang berasal dari user sebelum dipakai]` | `[Bagian Dasar Teori Ringkas dan Desain Teknis]` |
| `[Mendesain kontrak frame syscall (nr, arg0–arg5, ret) beserta enumerasi nomor syscall dan kode status yang konsisten antara C dan assembly]` | `[Isi include/mcsos/syscall.h]` |
| `[Mengimplementasikan mcsos_syscall_init, mcsos_syscall_set_user_region, mcsos_user_check_range, mcsos_copy_from_user, mcsos_syscall_dispatch, dan mcsos_syscall_dispatch_frame dalam C17 freestanding]` | `[Isi kernel/syscall/syscall.c, output make m10-host-test]` |
| `[Menulis stub INT 0x80 dalam x86_64 assembly yang menyimpan argumen register ke frame dan mengembalikan nilai lewat rax sebelum iretq]` | `[kernel/syscall/syscall_entry.S dan hasil objdump]` |
| `[Menyusun host unit test syscall yang menguji dispatch seluruh nomor syscall, validasi rentang pointer user, dan kode error MCSOS_ENOSYS/MCSOS_EFAULT]` | `[tests/test_syscall_host.c dan output "M10 syscall host tests passed"]` |
| `[Melakukan audit freestanding object dengan nm, readelf, dan objdump untuk syscall.o, syscall_entry.o, dan objek gabungan]` | `[build/m10/nm_undefined.txt, build/m10/readelf_header.txt, build/m10/objdump.txt]` |
| `[Mengintegrasikan subsistem syscall ke kernel MCSOS, membangun image ISO dengan Limine, dan memverifikasi log serial QEMU menunjukkan smoke test syscall berjalan]` | `[Perubahan kernel/core/kmain.c, build/mcsos.iso, build/qemu-serial.log]` |
| `[Mendiagnosis dan memperbaiki kerusakan Makefile (missing separator akibat .RECIPEPREFIX kustom) serta kekurangan include path pada flag kompilasi freestanding]` | `[Bagian 15 Debugging dan Failure Modes]` |
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
| M6 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M7 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M8 | Kernel heap allocator (kmem): free-list, split, coalesce, statistik heap | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M9 | Kernel thread dan cooperative scheduler | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M10 | Syscall ABI, dispatcher, dan antarmuka INT 0x80 | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M11 | Networking stack, packet parsing, UDP/TCP subset | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M12 | Security model, capability/ACL, syscall fuzzing, hardening | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M13 | SMP, scalability, lock stress, NUMA-aware preparation | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M14 | Framebuffer, graphics console, visual regression | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M15 | Virtualization/container subset | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ x ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Batas cakupan praktikum:

```text
Praktikum M10 berfokus pada perancangan dan implementasi antarmuka syscall ABI (mcsos syscall) sebagai lapisan kontrak antara mode pengguna dan mode kernel, dibangun di atas kernel heap (M8) dan scheduler kooperatif (M9) yang telah tersedia. Aktivitas mencakup penyusunan header kontrak frame syscall, implementasi dispatcher berbasis tabel fungsi, validasi rentang pointer user, penulisan stub assembly untuk vektor interrupt 0x80, host unit test, audit objek freestanding, integrasi ke kmain(), pembuatan image ISO dengan Limine, dan verifikasi log serial hasil boot QEMU. Penamaan resmi milestone M7 pada peta roadmap umum ("Syscall ABI dan user program loader") tumpang tindih secara konsep dengan penugasan M10 aktual yang diberikan pada semester berjalan (syscall ABI dan INT 0x80); laporan ini mengikuti panduan M10 aktual yang diberikan, bukan penomoran generik pada tabel roadmap. Praktikum ini belum mencakup user program loader/ELF loader mode pengguna sesungguhnya, transisi ring3→ring0 yang benar-benar diuji lewat instruksi int $0x80 dari kode user (vektor 0x80 pada IDT baru disiapkan tetapi belum diaktifkan pada boot path yang diuji), maupun syscall tambahan seperti file I/O, memory mapping, atau proses/fork.
```

---

## 6. Dasar Teori Ringkas

Tuliskan teori yang langsung diperlukan untuk memahami praktikum. Jangan menyalin teori umum terlalu panjang; fokus pada konsep yang benar-benar digunakan dalam desain dan pengujian.

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Praktikum M10 berfokus pada mekanisme system call (syscall), yaitu antarmuka terkontrol yang memungkinkan kode mode pengguna meminta layanan dari kernel tanpa mengakses memori atau register privileged secara langsung. Konsep yang diuji meliputi ABI syscall (kontrak nomor syscall, argumen, dan nilai kembali yang stabil), dispatcher syscall (pemetaan nomor syscall ke fungsi penangan lewat tabel), validasi pointer user (mencegah kernel mendereferensi alamat di luar region yang diizinkan atau alamat yang membuat integer overflow saat dijumlahkan dengan panjang buffer), dan mekanisme software interrupt (INT 0x80) sebagai titik masuk klasik dari mode pengguna ke mode kernel pada x86, dibandingkan dengan pendekatan modern seperti instruksi SYSCALL/SYSRET.
```

### 6.2 Konsep Arsitektur x86_64 yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[ELF64 x86_64 relocatable object]` | `[Format objek syscall.o dan syscall_entry.o hasil kompilasi freestanding sebelum digabung (ld -r) menjadi m10_syscall_combined.o]` | `[Output readelf -h build/m10/m10_syscall_combined.o dan build/kernel.elf]` |
| `[Konvensi passing argumen syscall Linux-like: rax=nr, rdi/rsi/rdx/r10/r8/r9=arg0..arg5]` | `[Dipakai sebagai dasar pemetaan register ke field mcsos_syscall_frame_t pada x86_64_syscall_int80_stub]` | `[Isi kernel/syscall/syscall_entry.S: movq %rax,0(%rsp) s.d. movq %r9,48(%rsp)]` |
| `[Software interrupt / gate IDT (INT n, iretq)]` | `[Vektor 0x80 disiapkan sebagai gate interrupt (X86_64_IDT_GATE_INTERRUPT) menuju x86_64_syscall_int80_stub, yang mengakhiri eksekusi dengan iretq untuk kembali ke mode pemanggil]` | `[Pemanggilan x86_64_idt_set_gate(0x80, ...) pada kmain.c (saat ini di dalam blok #if 0) dan instruksi iretq pada disassembly]` |
| `[Freestanding Environment]` | `[syscall.c dan syscall_entry.S tidak boleh bergantung pada malloc/free/printf libc host]` | `[Flag -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -mno-red-zone pada target freestanding M10]` |
| `[ABI x86_64 System V, stack alignment]` | `[Dasar pemanggilan mcsos_syscall_dispatch_frame(frame) dari x86_64_syscall_int80_stub dengan rdi menunjuk frame di stack]` | `[Output objdump -d pada x86_64_syscall_int80_stub]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding untuk kernel/syscall/syscall.c, x86_64 AT&T assembly untuk kernel/syscall/syscall_entry.S]` |
| Runtime | `[tanpa hosted libc pada jalur freestanding; host unit test tests/test_syscall_host.c memakai libc host untuk assert/puts/memcmp]` |
| ABI | `[x86_64 System V, konvensi argumen syscall gaya Linux]` |
| Compiler/assembler flags kritis | `[-ffreestanding, -fno-builtin, -fno-stack-protector, -fno-pic, -fno-pie, -mno-red-zone, --target=x86_64-unknown-none-elf, -Iinclude]` |
| Risiko undefined behavior | `[Integer overflow saat menjumlahkan addr+len pada validasi rentang user, pointer NULL dari user, panjang buffer write_serial berlebihan, ketidaksesuaian offset frame antara C dan assembly, dan potensi race jika dispatcher dipanggil dari interrupt context tanpa proteksi]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[Mechanism: Limited Direct Execution (OSTEP)]` | `[Konsep system call sebagai transisi terkontrol mode pengguna ke mode kernel lewat trap]` | `[Dasar desain x86_64_syscall_int80_stub dan mcsos_syscall_dispatch_frame]` |
| `[2]` | `[Intel/AMD manual: interrupt gate dan instruksi iretq]` | `[Struktur IDT gate dan tata cara kembali dari interrupt]` | `[Dasar pemasangan vektor 0x80 pada x86_64_idt_set_gate]` |
| `[3]` | `[Dokumentasi GNU Binutils]` | `[nm, readelf, dan objdump]` | `[Digunakan untuk memverifikasi objek freestanding syscall dan stub INT 0x80]` |
| `[4]` | `[Materi prasyarat M10: Kontrak ABI syscall, validasi pointer user, dispatch table, dan stub assembly x86_64]` | `[Seluruh bagian prasyarat teori M10]` | `[Menjadi dasar penetapan invariant dan security boundary syscall]` |
| `[5]` | `[Dokumentasi Limine bootloader dan xorriso]` | `[Pembuatan ISO hybrid BIOS/UEFI dan instalasi stage Limine]` | `[Digunakan untuk membangun build/mcsos.iso dan menjalankan boot QEMU]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows 10/11 x64]` |
| Lingkungan build | `[WSL 2 Ubuntu]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-unknown-none-elf]` |
| Emulator | `[QEMU — berhasil dijalankan pada sesi M10 ini dengan image build/mcsos.iso]` |
| Firmware emulator | `[OVMF (OVMF_CODE_4M.fd / OVMF_VARS.fd), boot UEFI lewat Limine]` |
| Debugger | `[GNU GDB — tersedia tetapi tidak dipakai pada sesi M10 ini]` |
| Build system | `[GNU Make 4.4.1]` |
| Bahasa utama | `[C17 Freestanding]` |
| Assembly | `[x86_64 AT&T assembly untuk syscall_entry.S]` |
| Bootloader/ISO | `[Limine (third_party/limine), xorriso untuk membangun build/mcsos.iso]` |

### 7.2 Versi Toolchain

Tempel output versi toolchain berikut.

```bash
clang --version | head -n 1
gcc --version | head -n 1
ld --version | head -n 1
nm --version | head -n 1
readelf --version | head -n 1
objdump --version | head -n 1
qemu-system-x86_64 --version
gdb --version | head -n 1
```

Output:

```text
Ubuntu clang version 21.1.8 (6ubuntu1)
gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
GNU ld (GNU Binutils for Ubuntu) 2.46
GNU nm (GNU Binutils for Ubuntu) 2.46
GNU readelf (GNU Binutils for Ubuntu) 2.46
GNU objdump (GNU Binutils for Ubuntu) 2.46
QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
GNU gdb (Ubuntu 17.1-2ubuntu1) 17.1
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[/home/syifa/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[praktikum/m10-syscall-abi]` |
| Commit hash awal (baseline sebelum branch M10 dibuka) | `` `[4453b81 — M9: implement cooperative kernel scheduler]` `` |
| Commit hash akhir | `` `[889411f]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

```text
mcsos/
├── include/
│   └── mcsos/
│       ├── kmem.h
│       ├── mcsos_thread.h
│       └── syscall.h
├── kernel/
│   ├── core/
│   │   └── kmain.c
│   ├── include/
│   │   └── mcsos/
│   │       └── mcsos_thread.h
│   ├── syscall/
│   │   ├── syscall.c
│   │   └── syscall_entry.S
│   └── mcsos_thread.c
├── tests/
│   ├── test_syscall.c
│   └── test_syscall_host.c
├── build/
│   ├── m10/
│   ├── kernel.elf
│   └── mcsos.iso
├── iso_root/
│   ├── boot/limine/
│   └── EFI/BOOT/
├── third_party/limine/
├── configs/limine/limine.conf
├── tools/scripts/make_iso.sh
├── tools/scripts/run_qemu.sh
├── logs/
└── Makefile
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[include/mcsos/syscall.h]` | `[baru]` | `[Deklarasi kontrak ABI syscall: MCSOS_SYSCALL_ABI_VERSION, MCSOS_SYSCALL_MAX_ARGS, mcsos_syscall_nr_t, mcsos_syscall_status_t, mcsos_syscall_frame_t, mcsos_user_region_t, mcsos_syscall_ops_t, dan seluruh deklarasi fungsi dispatcher/validasi]` | `[tinggi — kontrak ini dipakai bersama oleh C (syscall.c, kmain.c) dan assembly (syscall_entry.S)]` |
| `[kernel/syscall/syscall.c]` | `[baru]` | `[Implementasi dispatcher: mcsos_syscall_init, mcsos_syscall_set_user_region, mcsos_user_check_range, mcsos_copy_from_user, lima fungsi sys_* (ping, get_ticks, write_serial, yield, exit_thread), tabel g_table, mcsos_syscall_dispatch, dan mcsos_syscall_dispatch_frame]` | `[tinggi]` |
| `[kernel/syscall/syscall_entry.S]` | `[baru]` | `[Stub x86_64_syscall_int80_stub: memindahkan rax/rdi/rsi/rdx/r10/r8/r9 ke mcsos_syscall_frame_t di stack, memanggil mcsos_syscall_dispatch_frame, mengembalikan hasil ke rax, lalu iretq]` | `[tinggi — kesalahan offset frame dapat merusak nilai argumen/return]` |
| `[tests/test_syscall_host.c]` | `[baru]` | `[Host unit test lengkap: ping, get_ticks, write_serial, copy_from_user valid/tidak valid, yield, exit_thread, nomor syscall tidak dikenal, dan dispatch lewat frame]` | `[sedang]` |
| `[tests/test_syscall.c]` | `[baru, tidak dipakai Makefile]` | `[Draf host test awal yang lebih sederhana; digantikan oleh test_syscall_host.c sebagai test resmi pada target Makefile]` | `[rendah — berpotensi membingungkan karena dua file test serupa]` |
| `[Makefile]` | `[ubah]` | `[Menambahkan M10_BUILD, target m10-all/m10-host-test/m10-audit/m10-clean, aturan build test_syscall_host, syscall.o, syscall_entry.o, dan m10_syscall_combined.o; menambahkan -Iinclude pada COMMON_CFLAGS dan COMMON_ASFLAGS]` | `[tinggi — Makefile sempat rusak berkali-kali akibat .RECIPEPREFIX kustom yang hilang]` |
| `[kernel/core/kmain.c]` | `[ubah]` | `[Menambahkan #include syscall.h, extern x86_64_syscall_int80_stub, fungsi k_get_ticks/k_yield_current/k_exit_current/k_write_serial sebagai mcsos_syscall_ops_t, definisi MCSOS_USER_BASE/LIMIT, pemanggilan mcsos_syscall_init dan mcsos_syscall_set_user_region, fungsi m10_syscall_smoke_direct() yang memanggil dispatcher langsung, serta blok pemasangan gate IDT 0x80 yang saat ini dinonaktifkan dengan #if 0]` | `[sedang — bagian pemasangan gate 0x80 belum aktif pada boot path yang diuji]` |
| `[build/mcsos.iso]` | `[baru, artefak]` | `[Image bootable BIOS+UEFI hasil tools/scripts/make_iso.sh memakai Limine dan xorriso, dipakai untuk boot QEMU]` | `[rendah]` |
| `[logs/, M10.txt]` | `[baru]` | `[Transkrip mentah sesi kerja dan hash SHA-256 artefak build/m10]` | `[rendah — belum dirapikan/di-commit]` |

### 8.3 Ringkasan Diff

```bash
git status --short
git add Makefile kernel/core/kmain.c \
        include/mcsos/syscall.h \
        kernel/include/mcsos/mcsos_thread.h \
        kernel/syscall \
        tests/test_syscall.c \
        tests/test_syscall_host.c
git commit -m "M10: implement syscall ABI and INT80 interface"
git push -u origin praktikum/m10-syscall-abi
```

Output:

```text
[praktikum/m10-syscall-abi 889411f] M10: implement syscall ABI and INT80 interface
 8 files changed, 663 insertions(+), 7 deletions(-)
 create mode 100644 include/mcsos/syscall.h
 create mode 100644 kernel/include/mcsos/mcsos_thread.h
 create mode 100644 kernel/syscall/syscall.c
 create mode 100644 kernel/syscall/syscall_entry.S
 create mode 100644 tests/test_syscall.c
 create mode 100644 tests/test_syscall_host.c
...
 * [new branch]      praktikum/m10-syscall-abi -> praktikum/m10-syscall-abi
branch 'praktikum/m10-syscall-abi' set up to track 'origin/praktikum/m10-syscall-abi'.
```

Catatan: file `M10.txt`, `Makefile.bak`, `include/mcsos/mcsos_thread.h.bak`, dan direktori `logs/` masih berstatus *untracked* pada akhir sesi dan belum ikut dikomit (lihat Bagian 20 Known Issues).

---

## 9. Desain Teknis

### 9.1 Kontrak ABI Syscall

```text
Kontrak ABI syscall MCSOS versi 1 (MCSOS_SYSCALL_ABI_VERSION = 1u) mendefinisikan maksimum 6 argumen per syscall (MCSOS_SYSCALL_MAX_ARGS = 6u). Lima nomor syscall didefinisikan: MCSOS_SYS_PING (0), MCSOS_SYS_GET_TICKS (1), MCSOS_SYS_WRITE_SERIAL (2), MCSOS_SYS_YIELD (3), dan MCSOS_SYS_EXIT_THREAD (4), dengan MCSOS_SYS_MAX (5) sebagai batas atas. Kode status memakai konvensi mirip errno POSIX bernilai negatif: MCSOS_OK (0), MCSOS_EINVAL (-22), MCSOS_ENOSYS (-38), MCSOS_EFAULT (-14), MCSOS_EPERM (-1), MCSOS_EBUSY (-16).
```

### 9.2 Alur Panggilan Syscall

```text
Dua jalur pemanggilan disediakan pada M10: (1) jalur langsung dari kode kernel (dipakai oleh m10_syscall_smoke_direct pada kmain.c dan oleh host unit test) yang memanggil mcsos_syscall_dispatch(nr, arg0..arg5) secara langsung tanpa melalui interrupt; dan (2) jalur INT 0x80 yang dirancang untuk dipakai oleh kode mode pengguna di masa depan: instruksi "int $0x80" akan menjebak CPU ke x86_64_syscall_int80_stub, yang menyusun mcsos_syscall_frame_t di stack dari register rax/rdi/rsi/rdx/r10/r8/r9, memanggil mcsos_syscall_dispatch_frame(frame), menaruh frame->ret kembali ke rax, lalu iretq. Pada sesi ini, jalur (1) telah diuji end-to-end baik pada host unit test maupun pada boot QEMU nyata (log "[M10] syscall ping ok" dan "[M10] syscall get_ticks ok"), sedangkan jalur (2) baru diverifikasi lewat audit statis objek assembly (objdump menunjukkan simbol x86_64_syscall_int80_stub dan instruksi iretq) karena pemasangan gate IDT vektor 0x80 pada kmain.c masih dibungkus blok "#if 0" sehingga belum benar-benar dipasang saat kernel di-boot.
```

### 9.3 Diagram Alur (Deskripsi Tekstual)

```text
[Kode kernel/mode pengguna]
        |
        | (arg via register: rax=nr, rdi=arg0, rsi=arg1, rdx=arg2, r10=arg3, r8=arg4, r9=arg5)
        v
   int $0x80  --(trap, gate interrupt vektor 0x80 pada IDT)-->  x86_64_syscall_int80_stub
        |                                                              |
        |                                                   susun mcsos_syscall_frame_t di stack
        |                                                              |
        |                                                   call mcsos_syscall_dispatch_frame(frame)
        |                                                              |
        |                                                   frame->ret = mcsos_syscall_dispatch(nr,...)
        |                                                              |
        |                                                   nr valid? -> g_table[nr](args) : MCSOS_ENOSYS
        |                                                              |
        <-------------------------- rax = frame->ret, iretq ----------
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[mcsos_syscall_init(ops)]` | `[kmain]` | `[syscall.c]` | `[ops boleh NULL untuk memakai default]` | `[g_ops terisi; write_serial default berupa fungsi no-op yang mengembalikan len]` | `[Tidak ada error path eksplisit, seluruh field yang NULL diabaikan]` |
| `[mcsos_syscall_set_user_region(region)]` | `[kmain]` | `[syscall.c]` | `[region.base < region.limit untuk region valid]` | `[g_user_region diperbarui, dipakai oleh mcsos_user_check_range]` | `[Tidak ada validasi eksplisit pada fungsi ini sendiri]` |
| `[mcsos_user_check_range(addr, len)]` | `[sys_write_serial, mcsos_copy_from_user]` | `[syscall.c]` | `[g_user_region sudah diset]` | `[Return 1 jika [addr, addr+len) seluruhnya berada dalam [base, limit); 0 jika tidak]` | `[Return 0 pada len==0 dianggap valid (early return 1), pada base/limit tidak valid, pada addr di luar rentang, atau pada overflow addr+len]` |
| `[mcsos_copy_from_user(dst, src, len)]` | `[kode kernel yang perlu menyalin data dari user]` | `[syscall.c]` | `[dst dan src tidak NULL bila len>0]` | `[Menyalin len byte dari src ke dst setelah lolos mcsos_user_check_range]` | `[Return MCSOS_EINVAL bila dst/src NULL, MCSOS_EFAULT bila rentang tidak valid]` |
| `[mcsos_syscall_dispatch(nr, arg0..arg5)]` | `[m10_syscall_smoke_direct, x86_64_syscall_int80_stub via dispatch_frame, host test]` | `[syscall.c]` | `[tidak ada]` | `[Return nilai fungsi sys_* yang sesuai]` | `[Return MCSOS_ENOSYS bila nr >= MCSOS_SYS_MAX atau slot tabel kosong]` |
| `[mcsos_syscall_dispatch_frame(frame)]` | `[x86_64_syscall_int80_stub]` | `[syscall.c]` | `[frame tidak NULL]` | `[frame->ret diisi hasil mcsos_syscall_dispatch]` | `[Fungsi diam-diam kembali bila frame NULL]` |
| `[x86_64_syscall_int80_stub]` | `[CPU lewat gate IDT vektor 0x80]` | `[syscall_entry.S]` | `[IDT vektor 0x80 sudah dipasang menunjuk stub ini]` | `[rax berisi frame->ret, eksekusi kembali ke pemanggil lewat iretq]` | `[Tidak ada penanganan error eksplisit di level assembly]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `mcsos_syscall_frame_t` `` | `[nr, arg0, arg1, arg2, arg3, arg4, arg5, ret (seluruhnya uint64_t/int64_t)]` | `[Dialokasikan di stack oleh x86_64_syscall_int80_stub, atau di stack lokal pemanggil pada host test]` | `[selama satu transaksi syscall]` | `[Offset field (0,8,16,24,32,40,48,56 byte) harus sama persis dengan urutan movq pada syscall_entry.S]` |
| `` `mcsos_user_region_t` `` | `[base, limit (uintptr_t)]` | `[Variabel global g_user_region pada syscall.c, diset sekali lewat kmain.c]` | `[selama kernel berjalan]` | `[base < limit untuk region yang dianggap valid; base==0 atau limit<=base dianggap belum diinisialisasi]` |
| `` `mcsos_syscall_ops_t` `` | `[get_ticks, yield_current, exit_current, write_serial (function pointer)]` | `[Variabel global g_ops pada syscall.c, diisi dari kmain.c: k_get_ticks, k_yield_current, k_exit_current, k_write_serial]` | `[selama kernel berjalan]` | `[write_serial tidak pernah NULL (memakai default_write_serial bila tidak diberikan); get_ticks/yield_current/exit_current NULL berarti MCSOS_EBUSY saat dipanggil]` |
| `` `g_table[MCSOS_SYS_MAX]` `` | `[array pointer fungsi syscall_fn_t]` | `[statis pada syscall.c]` | `[sepanjang program]` | `[Indeks harus sama dengan nilai enum mcsos_syscall_nr_t yang bersangkutan]` |

### 9.6 Invariants

Tuliskan invariant yang harus benar sepanjang eksekusi.

1. `Nomor syscall (nr) yang lebih besar atau sama dengan MCSOS_SYS_MAX selalu ditolak dengan MCSOS_ENOSYS sebelum menyentuh tabel g_table.`
2. `Setiap entri g_table[0..MCSOS_SYS_MAX-1] harus berupa pointer fungsi valid (tidak NULL); bila suatu saat ada slot kosong, dispatcher tetap mengembalikan MCSOS_ENOSYS, bukan crash.`
3. `Setiap rentang pointer [addr, addr+len) yang berasal dari user harus seluruhnya berada di dalam [g_user_region.base, g_user_region.limit) sebelum dibaca oleh kernel, termasuk pemeriksaan agar addr+len tidak overflow (last < addr dianggap invalid).`
4. `MCSOS_SYS_WRITE_SERIAL menolak panjang buffer lebih dari 4096 byte (len > 4096u) sebelum memanggil g_ops.write_serial.`
5. `Urutan penyimpanan register pada x86_64_syscall_int80_stub (rax→nr, rdi→arg0, rsi→arg1, rdx→arg2, r10→arg3, r8→arg4, r9→arg5) harus konsisten dengan urutan field mcsos_syscall_frame_t agar dispatcher membaca argumen yang benar.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[g_ops (mcsos_syscall_ops_t)]` | `[kernel/syscall/syscall.c]` | `[none]` | `[Ya, sebagai bagian dari x86_64_syscall_int80_stub bila diaktifkan]` | `[Diisi sekali saat boot lewat mcsos_syscall_init; tidak ada proteksi terhadap penulisan ulang bersamaan]` |
| `[g_user_region]` | `[kernel/syscall/syscall.c]` | `[none]` | `[Ya]` | `[Diisi sekali saat boot lewat mcsos_syscall_set_user_region; belum mendukung banyak proses dengan region berbeda]` |
| `[g_table]` | `[kernel/syscall/syscall.c]` | `[none, konstan setelah kompilasi]` | `[Ya]` | `[Tabel statis, tidak diubah saat runtime]` |

Lock order yang berlaku:

```text
Pada M10 belum terdapat mekanisme locking karena dispatcher syscall dirancang untuk dipanggil secara tunggal (single-threaded) baik dari kmain() maupun (secara desain, belum diaktifkan) dari interrupt context INT 0x80. Belum ada proteksi terhadap pemanggilan syscall yang bersamaan dari lebih dari satu CPU/thread, sehingga integrasi dengan scheduler preemptive (M9 lanjutan) atau SMP memerlukan locking/atomic tambahan yang belum diimplementasikan pada milestone ini.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Pointer user NULL atau di luar region pada MCSOS_SYS_WRITE_SERIAL]` | `[sys_write_serial]` | `[Cek ptr==0 mengembalikan MCSOS_EINVAL, lalu mcsos_user_check_range mengembalikan MCSOS_EFAULT bila di luar rentang]` | `[Logika pada kernel/syscall/syscall.c baris sys_write_serial]` |
| `[Integer overflow saat menjumlahkan addr+len pada validasi rentang]` | `[mcsos_user_check_range]` | `[Pemeriksaan eksplisit "if (last < addr) return 0;" untuk mendeteksi wraparound]` | `[Isi kernel/syscall/syscall.c]` |
| `[Panjang buffer write_serial berlebihan]` | `[sys_write_serial]` | `[Pembatasan len > 4096u ditolak dengan MCSOS_EINVAL sebelum menyentuh g_ops.write_serial]` | `[Isi kernel/syscall/syscall.c]` |
| `[Ketidaksesuaian offset frame antara C dan assembly]` | `[syscall_entry.S vs mcsos_syscall_frame_t]` | `[Offset ditulis manual (0,8,16,24,32,40,48,56) mengikuti urutan field frame, diverifikasi lewat objdump]` | `[Disassembly build/m10/objdump.txt menunjukkan movq berurutan sesuai offset]` |
| `[Kernel gagal build karena header mcsos/syscall.h tidak ditemukan saat kompilasi freestanding penuh]` | `[kernel/syscall/syscall.c dan kernel/core/kmain.c]` | `[Menambahkan flag -Iinclude pada COMMON_CFLAGS dan COMMON_ASFLAGS Makefile]` | `[Log build sebelum dan sesudah perbaikan, lihat Bagian 15]` |
| `[Gate IDT vektor 0x80 belum benar-benar dipasang pada boot path yang diuji]` | `[kernel/core/kmain.c, blok #if 0 di sekitar x86_64_idt_set_gate(0x80, ...)]` | `[Diverifikasi lewat audit statis objdump pada stub, bukan lewat eksekusi int $0x80 nyata]` | `[Log serial QEMU tidak memuat "[M10] syscall vector 0x80 installed" karena baris tersebut berada dalam blok yang dinonaktifkan]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[Argumen syscall dari mode pengguna (dirancang, belum aktif) atau dari pemanggil kernel]` | `[Nomor syscall (nr) dan argumen pointer/panjang]` | `[Cek nr < MCSOS_SYS_MAX, cek pointer/panjang lewat mcsos_user_check_range sebelum dipakai]` | `[Return MCSOS_ENOSYS/MCSOS_EINVAL/MCSOS_EFAULT, tidak melanjutkan operasi tidak aman]` |
| `[Region memori pengguna (MCSOS_USER_BASE=0x400000 s.d. MCSOS_USER_LIMIT=0x800000000000)]` | `[Alamat dan panjang buffer yang diklaim berasal dari user]` | `[mcsos_user_check_range memeriksa rentang penuh termasuk potensi overflow]` | `[Return 0/MCSOS_EFAULT, kernel tidak mendereferensi pointer di luar region]` |
| `[Build system Makefile]` | `[Target M10 yang ditambahkan manual]` | `[Uji make m10-host-test dan make m10-audit sebelum diintegrasikan ke build kernel penuh]` | `[Build dihentikan bila host unit test syscall gagal atau assertion audit (grep -q) tidak terpenuhi]` |

---

## 10. Langkah Kerja Implementasi

Gunakan tabel berikut untuk setiap langkah. Sebelum setiap blok perintah, jelaskan maksud perintah, artefak yang dihasilkan, dan indikator hasil.

### Langkah 1 — `Preflight, Pembersihan Sisa Sesi Sebelumnya, dan Verifikasi Toolchain`

Maksud langkah:

```text
Memverifikasi direktori kerja, branch aktif (m9-kernel-thread-scheduler), status git, dan versi toolchain sebelum memulai M10. Ditemukan beberapa file sampah untracked (mis. "-DMCSOS_HOST_TEST", "-Iinclude", "-Wall") akibat kesalahan paste multi-baris pada sesi sebelumnya, yang perlu dibersihkan terlebih dahulu.
```

Perintah:

```bash
cd ~/src/mcsos
pwd
git branch --show-current
git status --short
rm -- -DMCSOS_HOST_TEST -Iinclude -Wall -c -fsyntax-only -o -std=c17
clang --version
gcc --version
ld --version | head -n 1
nm --version | head -n 1
readelf --version | head -n 1
objdump --version | head -n 1
qemu-system-x86_64 --version
gdb --version | head -n 1
```

Output ringkas:

```text
/home/syifa/src/mcsos
m9-kernel-thread-scheduler
?? -DMCSOS_HOST_TEST
?? -Iinclude
?? -Wall
?? -c
?? -fsyntax-only
?? -o
?? -std=c17
?? include/mcsos/mcsos_thread.h.bak
?? kernel/include/mcsos/mcsos_thread.h
(setelah rm --) git status --short hanya menyisakan kedua file .h di atas
Ubuntu clang version 21.1.8 (6ubuntu1)
gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
GNU ld (GNU Binutils for Ubuntu) 2.46
QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
GNU gdb (Ubuntu 17.1-2ubuntu1) 17.1
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Working tree bersih dari file sampah paste]` | `[~/src/mcsos]` | `[Baseline bersih sebelum branch M10 dibuka]` |

Indikator berhasil:

```text
File sampah bernama seperti flag compiler (mis. "-DMCSOS_HOST_TEST") berhasil dihapus dengan "rm --" untuk menghindari file tersebut ditafsirkan sebagai opsi command, dan seluruh versi toolchain tercatat sebelum perubahan M10 dimulai.
```

### Langkah 2 — `Membuka Branch Baru dan Menyusun Struktur Direktori Syscall`

Maksud langkah:

```text
Membuat branch praktikum/m10-syscall-abi dari branch m9-kernel-thread-scheduler, lalu menyiapkan direktori kerja untuk header, implementasi, test, dan skrip M10.
```

Perintah:

```bash
git checkout -b praktikum/m10-syscall-abi
mkdir -p include/mcsos kernel/syscall tests scripts logs
git branch --show-current
find kernel -maxdepth 2 -type d | grep syscall
find . -maxdepth 2 -type d | grep tests
```

Output ringkas:

```text
Switched to a new branch 'praktikum/m10-syscall-abi'
praktikum/m10-syscall-abi
kernel/syscall
./tests
./tests/toolchain
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Branch kerja baru]` | `[praktikum/m10-syscall-abi]` | `[Isolasi pengerjaan M10 dari branch M9]` |
| `[Direktori kernel/syscall]` | `[kernel/syscall/]` | `[Lokasi implementasi dispatcher dan stub INT 0x80]` |

Indikator berhasil:

```text
Branch praktikum/m10-syscall-abi aktif dan struktur direktori kernel/syscall siap dipakai untuk file implementasi berikutnya.
```

### Langkah 3 — `Menulis Header Kontrak ABI Syscall`

Maksud langkah:

```text
Menulis include/mcsos/syscall.h berisi versi ABI, nomor syscall, kode status, struktur frame syscall, region user, dan tabel operasi kernel (mcsos_syscall_ops_t), beserta deklarasi seluruh fungsi dispatcher dan validasi.
```

Perintah:

```bash
cat > include/mcsos/syscall.h <<'EOF'
... (lihat isi lengkap pada Bagian 9.1 dan file repository)
EOF
grep -n "MCSOS_SYS_" include/mcsos/syscall.h
grep -n "mcsos_syscall_dispatch" include/mcsos/syscall.h
wc -l include/mcsos/syscall.h
```

Output ringkas:

```text
11:    MCSOS_SYS_PING = 0,
12:    MCSOS_SYS_GET_TICKS = 1,
13:    MCSOS_SYS_WRITE_SERIAL = 2,
14:    MCSOS_SYS_YIELD = 3,
15:    MCSOS_SYS_EXIT_THREAD = 4,
16:    MCSOS_SYS_MAX = 5
56:int64_t mcsos_syscall_dispatch(
65:void mcsos_syscall_dispatch_frame(
68 include/mcsos/syscall.h
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[syscall.h]` | `[include/mcsos/syscall.h]` | `[Kontrak ABI syscall lengkap, 68 baris]` |

Indikator berhasil:

```text
Header selesai ditulis dengan seluruh enumerasi, struct, dan deklarasi fungsi yang diperlukan; belum ada verifikasi sintaks pada langkah ini, dilakukan pada langkah berikutnya bersama implementasi.
```

### Langkah 4 — `Menulis Implementasi Dispatcher Syscall`

Maksud langkah:

```text
Menulis kernel/syscall/syscall.c berisi mcsos_syscall_init, mcsos_syscall_set_user_region, mcsos_user_check_range, mcsos_copy_from_user, lima fungsi sys_* (ping, get_ticks, write_serial, yield, exit_thread), tabel dispatch g_table, mcsos_syscall_dispatch, dan mcsos_syscall_dispatch_frame.
```

Perintah:

```bash
cat > kernel/syscall/syscall.c <<'EOF'
... (lihat isi lengkap pada Bagian 9 dan file repository)
EOF
grep -n "sys_ping" kernel/syscall/syscall.c
grep -n "g_table" kernel/syscall/syscall.c
grep -n "mcsos_syscall_dispatch" kernel/syscall/syscall.c
wc -l kernel/syscall/syscall.c
```

Output ringkas:

```text
49:static int64_t sys_ping(uint64_t a0, uint64_t a1, uint64_t a2,
91:    sys_ping,
90:static syscall_fn_t g_table[MCSOS_SYS_MAX] = {
98:int64_t mcsos_syscall_dispatch(uint64_t nr, uint64_t arg0, uint64_t arg1,
107:void mcsos_syscall_dispatch_frame(mcsos_syscall_frame_t *frame) {
112 kernel/syscall/syscall.c
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[syscall.c]` | `[kernel/syscall/syscall.c]` | `[Implementasi dispatcher syscall, 112 baris]` |

Indikator berhasil:

```text
File berisi 112 baris dengan seluruh fungsi sys_*, tabel g_table berisi 5 entri sesuai MCSOS_SYS_MAX, dan fungsi dispatch/dispatch_frame lengkap; kompilasi diverifikasi pada langkah host test berikutnya.
```

### Langkah 5 — `Menulis Stub Assembly INT 0x80`

Maksud langkah:

```text
Menulis kernel/syscall/syscall_entry.S berisi x86_64_syscall_int80_stub yang menyusun mcsos_syscall_frame_t dari register, memanggil mcsos_syscall_dispatch_frame, mengembalikan hasil ke rax, dan iretq. Setelah ditulis, objek dikompilasi secara mandiri untuk memverifikasi sintaks assembly sebelum diintegrasikan ke build penuh.
```

Perintah:

```bash
cat > kernel/syscall/syscall_entry.S <<'EOF'
... (lihat isi lengkap pada file repository)
EOF
grep -n "x86_64_syscall_int80_stub" kernel/syscall/syscall_entry.S
grep -n "iretq" kernel/syscall/syscall_entry.S
wc -l kernel/syscall/syscall_entry.S
clang --target=x86_64-unknown-none-elf -c kernel/syscall/syscall_entry.S -o /tmp/syscall_entry.o
objdump -d /tmp/syscall_entry.o | head -n 50
```

Output ringkas:

```text
2:.global x86_64_syscall_int80_stub
3:.type x86_64_syscall_int80_stub, @function
10:x86_64_syscall_int80_stub:
27:.size x86_64_syscall_int80_stub, . - x86_64_syscall_int80_stub
9:# caller-save inputs into mcsos_syscall_frame_t and returns through iretq.
25:    iretq
27 kernel/syscall/syscall_entry.S

0000000000000000 <x86_64_syscall_int80_stub>:
   0:   fc                      cld
   1:   48 83 ec 40             sub    $0x40,%rsp
   5:   48 89 04 24             mov    %rax,(%rsp)
   9:   48 89 7c 24 08          mov    %rdi,0x8(%rsp)
   e:   48 89 74 24 10          mov    %rsi,0x10(%rsp)
  13:   48 89 54 24 18          mov    %rdx,0x18(%rsp)
  18:   4c 89 54 24 20          mov    %r10,0x20(%rsp)
  1d:   4c 89 44 24 28          mov    %r8,0x28(%rsp)
  22:   4c 89 4c 24 30          mov    %r9,0x30(%rsp)
  27:   48 c7 44 24 38 00 00    movq   $0x0,0x38(%rsp)
  30:   48 89 e7                mov    %rsp,%rdi
  33:   e8 00 00 00 00          call   38 <x86_64_syscall_int80_stub+0x38>
  38:   48 8b 44 24 38          mov    0x38(%rsp),%rax
  3d:   48 83 c4 40             add    $0x40,%rsp
  41:   48 cf                   iretq
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[syscall_entry.S]` | `[kernel/syscall/syscall_entry.S]` | `[Stub INT 0x80, 27 baris]` |
| `[syscall_entry.o (percobaan)]` | `[/tmp/syscall_entry.o]` | `[Verifikasi awal kompilasi freestanding sebelum diintegrasikan ke Makefile]` |

Indikator berhasil:

```text
Kompilasi freestanding berhasil tanpa error, dan disassembly menunjukkan urutan penyimpanan register (rax, rdi, rsi, rdx, r10, r8, r9) sesuai offset 0,8,16,24,32,40,48, diikuti call ke mcsos_syscall_dispatch_frame, pengambilan nilai balik dari offset 0x38 (56) ke rax, dan iretq sebagai instruksi terakhir.
```

### Langkah 6 — `Menyusun Host Unit Test Syscall`

Maksud langkah:

```text
Menulis tests/test_syscall_host.c untuk menguji seluruh nomor syscall (ping, get_ticks, write_serial, yield, exit_thread), copy_from_user pada rentang valid dan tidak valid, MCSOS_ENOSYS untuk nomor syscall tidak dikenal, dan dispatch lewat mcsos_syscall_frame_t.
```

Perintah:

```bash
cat > tests/test_syscall_host.c <<'EOF'
... (lihat isi lengkap pada file repository)
EOF
grep -n "fake_ticks" tests/test_syscall_host.c
grep -n "mcsos_copy_from_user" tests/test_syscall_host.c
grep -n "M10 syscall host tests passed" tests/test_syscall_host.c
wc -l tests/test_syscall_host.c
```

Output ringkas:

```text
7:static uint64_t fake_ticks(void) { return 12345u; }
63:        mcsos_copy_from_user(
71:        mcsos_copy_from_user(
103:    puts("M10 syscall host tests passed");
106 tests/test_syscall_host.c
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[test_syscall_host.c]` | `[tests/test_syscall_host.c]` | `[Host unit test syscall, 106 baris]` |

Indikator berhasil:

```text
File selesai ditulis dengan seluruh skenario uji (ping, get_ticks, write_serial, copy_from_user valid/EFAULT, yield, exit_thread, nomor tidak dikenal, dan dispatch_frame); eksekusi sebenarnya dilakukan pada Langkah 8 setelah target Makefile ditambahkan.
```

### Langkah 7 — `Menambahkan Target M10 pada Makefile (Diagnosis Missing Separator)`

Maksud langkah:

```text
Menambahkan variabel M10_BUILD dan target m10-all, m10-host-test, m10-audit, m10-clean pada Makefile lewat heredoc, agar build host test, objek freestanding, penggabungan objek, dan audit dapat dijalankan otomatis.
```

Perintah (ringkasan, termasuk proses debugging):

```bash
cat >> Makefile <<'EOF'
... (target m10-all, m10-host-test, m10-audit, m10-clean)
EOF
make m10-host-test
sed -n '216,230p' Makefile
sed -n '216,230p' Makefile | cat -te
nl -ba Makefile | sed -n '220,226p'
sed -n '220,226p' Makefile | od -An -t x1
```

Output ringkas (kegagalan awal):

```text
Makefile:223: *** missing separator.  Stop.
```

Analisis:

```text
Pemeriksaan dengan cat -te dan od -An -tx1 menunjukkan baris resep (mis. "rm -rf $(M10_BUILD)") diawali karakter tab, bukan karakter '>' yang dipakai oleh seluruh resep lain pada Makefile (mis. ">mkdir -p $(dir $@)"). Ternyata Makefile ini memakai konvensi ".RECIPEPREFIX := >" (karakter '>' sebagai pengganti tab) yang sudah ada sejak M6/M8/M9, sehingga baris baru yang memakai tab tidak dikenali sebagai baris resep.
```

Perbaikan:

```bash
sed -i '2i .RECIPEPREFIX := >' Makefile
make clean
make
```

Output setelah baris `.RECIPEPREFIX := >` ditambahkan kembali di dekat awal file:

```text
mkdir -p build/normal/kernel/arch/x86_64/
clang --target=x86_64-unknown-none-elf ... -c kernel/arch/x86_64/idt.c -o build/normal/kernel/arch/x86_64/idt.o
...
ld.lld -nostdlib -static ... -o build/kernel.elf ...
readelf -h build/kernel.elf > build/kernel.readelf.header.txt
...
grep -q 'lidt' build/kernel.disasm.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Target Makefile M10]` | `[Makefile: M10_BUILD, m10-all, m10-host-test, m10-audit, m10-clean]` | `[Otomasi build, test, dan audit M10]` |

Indikator berhasil:

```text
Setelah baris .RECIPEPREFIX := > dipulihkan (sempat hilang saat resep M10 ditambahkan lewat heredoc dan sempat pula dicoba diperbaiki dengan mengganti tab menjadi '>' lewat "sed -i 's/^\t/>/' Makefile"), seluruh resep lama (M6/M8/M9) maupun baru (M10) kembali dikenali sebagai baris resep yang valid, dan "make" berhasil membangun kernel penuh tanpa error "missing separator".
```

### Langkah 8 — `Menjalankan Host Unit Test dan Audit Objek Freestanding M10`

Maksud langkah:

```text
Menjalankan make m10-host-test untuk mengompilasi dan mengeksekusi tests/test_syscall_host.c bersama kernel/syscall/syscall.c sebagai program host, lalu menjalankan make m10-audit untuk membangun objek freestanding syscall.o dan syscall_entry.o, menggabungkannya dengan ld -r, dan memverifikasi hasilnya dengan nm/readelf/objdump.
```

Perintah:

```bash
make m10-host-test
make m10-audit
cat build/m10/nm_undefined.txt
grep "Machine:" build/m10/readelf_header.txt
grep "x86_64_syscall_int80_stub" build/m10/objdump.txt
grep "iretq" build/m10/objdump.txt
```

Output ringkas:

```text
clang -std=c17 -Wall -Wextra -Werror -Iinclude tests/test_syscall_host.c kernel/syscall/syscall.c -o build/m10/test_syscall_host
./build/m10/test_syscall_host
M10 syscall host tests passed

clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -fno-pie -m64 -mno-red-zone -Wall -Wextra -Werror -Iinclude -c kernel/syscall/syscall.c -o build/m10/syscall.o
clang --target=x86_64-unknown-none-elf -c kernel/syscall/syscall_entry.S -o build/m10/syscall_entry.o
ld -r build/m10/syscall.o build/m10/syscall_entry.o -o build/m10/m10_syscall_combined.o
nm -u build/m10/m10_syscall_combined.o > build/m10/nm_undefined.txt
readelf -h build/m10/m10_syscall_combined.o > build/m10/readelf_header.txt
objdump -dr build/m10/m10_syscall_combined.o > build/m10/objdump.txt
(nm_undefined.txt kosong)
  Machine:                           Advanced Micro Devices X86-64
0000000000000548 <x86_64_syscall_int80_stub>:
 589:   48 cf                   iretq
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[test_syscall_host]` | `[build/m10/test_syscall_host]` | `[Executable host unit test syscall]` |
| `[m10_syscall_combined.o]` | `[build/m10/m10_syscall_combined.o]` | `[Objek gabungan syscall.o + syscall_entry.o]` |
| `[nm_undefined.txt, readelf_header.txt, objdump.txt]` | `[build/m10/]` | `[Bukti audit ELF64 x86_64, simbol stub, dan instruksi iretq]` |

Indikator berhasil:

```text
"./build/m10/test_syscall_host" mencetak "M10 syscall host tests passed"; build/m10/nm_undefined.txt kosong (tidak ada simbol undefined); readelf -h menunjukkan Machine "Advanced Micro Devices X86-64"; objdump.txt memuat simbol x86_64_syscall_int80_stub pada offset 0x548 dan instruksi iretq pada offset 0x589, seluruh assertion grep -q pada target m10-audit lulus tanpa error.
```

### Langkah 9 — `Mengintegrasikan Syscall ke kmain() dan Memperbaiki Include Path Freestanding`

Maksud langkah:

```text
Menambahkan #include mcsos/syscall.h, extern x86_64_syscall_int80_stub, definisi mcsos_syscall_ops_t berisi fungsi k_get_ticks/k_yield_current/k_exit_current/k_write_serial, region user MCSOS_USER_BASE/MCSOS_USER_LIMIT, pemanggilan mcsos_syscall_init dan mcsos_syscall_set_user_region, serta fungsi m10_syscall_smoke_direct() ke kernel/core/kmain.c, kemudian membangun kernel penuh.
```

Perintah:

```bash
nano kernel/core/kmain.c
make clean
make
```

Output ringkas (kegagalan awal):

```text
kernel/core/kmain.c:11:10: fatal error: 'mcsos/syscall.h' file not found
    11 | #include <mcsos/syscall.h>
1 error generated.
...
kernel/syscall/syscall.c:1:10: fatal error: 'mcsos/syscall.h' file not found
    1 | #include "mcsos/syscall.h"
1 error generated.
```

Analisis:

```text
COMMON_CFLAGS dan COMMON_ASFLAGS pada Makefile hanya memuat -Ikernel/arch/x86_64/include dan -Ikernel/include, belum memuat -Iinclude (lokasi include/mcsos/syscall.h), sehingga build freestanding penuh gagal walau file header sudah benar dan sudah bisa dikompilasi mandiri dengan "clang -Iinclude ...".
```

Perbaikan:

```bash
sed -i '22 s#$# -Iinclude#' Makefile
sed -i '23 s#$# -Iinclude#' Makefile
sed -n '20,28p' Makefile | cat -te
sed -i '24d' Makefile
make clean
make
```

Output setelah perbaikan:

```text
clang --target=x86_64-unknown-none-elf -std=c17 ... -Ikernel/arch/x86_64/include -Ikernel/include -Iinclude -c kernel/syscall/syscall.c -o build/normal/kernel/syscall/syscall.o
...
ld.lld -nostdlib -static -z max-page-size=0x1000 -T linker.ld -Map=build/kernel.map -o build/kernel.elf \
  ... build/normal/kernel/syscall/syscall.o ... build/normal/kernel/syscall/syscall_entry.o
readelf -h build/kernel.elf > build/kernel.readelf.header.txt
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[kernel.elf]` | `[build/kernel.elf]` | `[Kernel penuh dengan subsistem syscall M10 terintegrasi]` |
| `[kernel.syms.txt, kernel.disasm.txt]` | `[build/]` | `[Bukti simbol dan disassembly kernel penuh]` |

Indikator berhasil:

```text
Penambahan -Iinclude pada COMMON_CFLAGS dan COMMON_ASFLAGS (setelah satu baris duplikat hasil sed yang salah dihapus dengan "sed -i '24d'") menyelesaikan error "file not found", dan "make" berhasil membangun build/kernel.elf secara penuh, lengkap dengan seluruh assertion audit bawaan (ELF64, Machine X86-64, simbol kmain/x86_64_idt_init/x86_64_trap_dispatch, instruksi iretq/lidt) yang lulus.
```

Catatan mengenai gate IDT 0x80:

```text
Pada kmain.c, pemanggilan x86_64_idt_set_gate(0x80, (uint64_t)x86_64_syscall_int80_stub, X86_64_IDT_GATE_INTERRUPT) beserta log_writeln("[M10] syscall vector 0x80 installed") berada di dalam blok "#if 0 ... #endif", sehingga baris tersebut tidak benar-benar dieksekusi pada build ini. Ini adalah keterbatasan yang disadari dan dicatat sebagai known issue (Bagian 20), bukan kesalahan yang tidak terdeteksi: fungsi m10_syscall_smoke_direct() tetap menguji dispatcher lewat pemanggilan langsung mcsos_syscall_dispatch, bukan lewat instruksi int $0x80.
```

### Langkah 10 — `Menambahkan Smoke Test Syscall Langsung di kmain()`

Maksud langkah:

```text
Menambahkan pemanggilan MCSOS_SYS_GET_TICKS setelah MCSOS_SYS_PING pada m10_syscall_smoke_direct(), agar smoke test mencakup lebih dari satu syscall saat kernel benar-benar di-boot.
```

Perintah:

```bash
sed -i '/log_writeln("\[M10\] syscall ping ok");/a\
    r = mcsos_syscall_dispatch(\
        MCSOS_SYS_GET_TICKS,\
        0, 0, 0, 0, 0, 0);\
\
    if (r < 0) {\
        KERNEL_PANIC("M10 syscall get_ticks failed", r);\
    }\
\
    log_writeln("[M10] syscall get_ticks ok");\
    log_writeln("[M10] syscall smoke done");' kernel/core/kmain.c
make
```

Output ringkas:

```text
mkdir -p build/normal/kernel/core/
clang ... -c kernel/core/kmain.c -o build/normal/kernel/core/kmain.o
ld.lld ... -o build/kernel.elf ...
grep -q 'lidt' build/kernel.disasm.txt
(seluruh assertion audit bawaan lulus)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[kmain.c (diperbarui)]` | `[kernel/core/kmain.c]` | `[Smoke test PING dan GET_TICKS dijalankan berurutan saat boot]` |

Indikator berhasil:

```text
Kernel berhasil dibangun ulang tanpa error, dan smoke test lengkap (ping, get_ticks, "smoke done") siap diverifikasi lewat log serial QEMU pada langkah berikutnya.
```

### Langkah 11 — `Membangun Image ISO dengan Limine dan Menjalankan Boot QEMU`

Maksud langkah:

```text
Membangun image bootable build/mcsos.iso menggunakan xorriso dan bootloader Limine (third_party/limine), lalu menjalankan QEMU headless dengan serial diarahkan ke file log untuk memverifikasi bahwa kernel benar-benar boot dan menjalankan smoke test syscall pada mesin virtual nyata.
```

Perintah:

```bash
bash tools/scripts/make_iso.sh
bash tools/scripts/run_qemu.sh
cat build/qemu-serial.log
```

Output ringkas (percobaan pertama, ISO dibangun manual tanpa limine bios-install, gagal boot):

```text
ERROR: serial log kosong: build/qemu-serial.log
```

Output setelah memakai skrip resmi tools/scripts/make_iso.sh (memanggil "$LIMINE_DIR/limine" bios-install "$ISO"):

```text
'build/kernel.elf' -> 'iso_root/boot/kernel.elf'
...
Limine BIOS stages installed successfully.
e2e073df62edbb1b72a451452e3bd7d3ff15e4ba985a0c23a0f6b7c4047e47e6  build/mcsos.iso
OK: ISO dibuat pada build/mcsos.iso
```

Log serial hasil boot QEMU:

```text
limine: Loading executable `boot():/boot/kernel.elf`...
[MCSOS:M5] boot: external interrupt bring-up start
idt_base=0xffffffff80007000
idt_limit=0x0000000000000fff
[M4] IDT loaded
[MCSOS:M5] idt: loaded
[MCSOS:M5] pic: remapped; mask master=0x00000000000000fe slave=0x00000000000000ff
[MCSOS:M5] pit: configured 100Hz
[M8] kernel heap initialized
[M10] user region initialized
[M10] syscall subsystem initialized
[M10] syscall ping ok
[M10] syscall get_ticks ok
[M10] syscall smoke done
[M9] scheduler initialized
[MCSOS:M5] sti: enabling interrupts
[MCSOS:TIMER] ticks=0x0000000000000064
[MCSOS:TIMER] ticks=0x00000000000000c8
[MCSOS:TIMER] ticks=0x000000000000012c
[MCSOS:TIMER] ticks=0x0000000000000190
[MCSOS:TIMER] ticks=0x00000000000001f4
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[mcsos.iso]` | `[build/mcsos.iso]` | `[Image bootable BIOS+UEFI berisi kernel.elf dan Limine]` |
| `[qemu-serial.log]` | `[build/qemu-serial.log]` | `[Bukti log serial boot QEMU nyata, memuat smoke test syscall]` |

Indikator berhasil:

```text
Setelah image ISO dibangun lewat skrip resmi (yang menjalankan limine bios-install, tidak seperti percobaan manual pertama), QEMU berhasil boot dan log serial memuat pesan "[M10] user region initialized", "[M10] syscall subsystem initialized", "[M10] syscall ping ok", "[M10] syscall get_ticks ok", dan "[M10] syscall smoke done" secara berurutan sebelum scheduler M9 diinisialisasi dan interrupt timer mulai berjalan, membuktikan subsistem syscall M10 berfungsi pada runtime nyata lewat jalur pemanggilan langsung (bukan lewat int $0x80).
```

Catatan keterbatasan:

```text
Skrip tools/scripts/run_qemu.sh memiliki assertion bawaan di akhir (grep -q 'MCSOS 260502 M2 boot path entered', '[M2] early serial online', '[M2] kernel reached controlled halt loop') yang merupakan peninggalan milestone M2 dan tidak lagi relevan/tidak muncul pada log kernel M10 ini (kernel M10 tidak berhenti pada halt loop, melainkan terus berjalan menampilkan tick timer). Karena itu skrip tidak sampai mencetak baris "OK: QEMU serial log valid" meskipun boot dan smoke test syscall sesungguhnya berhasil; ini dicatat sebagai known issue pada Bagian 20, bukan kegagalan subsistem syscall.
```

### Langkah 12 — `Commit dan Push Perubahan M10`

Maksud langkah:

```text
Menyimpan seluruh perubahan header, implementasi, test, Makefile, dan kmain.c ke repository Git pada branch praktikum/m10-syscall-abi, lalu mendorongnya ke remote.
```

Perintah:

```bash
git add Makefile kernel/core/kmain.c \
        include/mcsos/syscall.h \
        kernel/include/mcsos/mcsos_thread.h \
        kernel/syscall \
        tests/test_syscall.c \
        tests/test_syscall_host.c
git commit -m "M10: implement syscall ABI and INT80 interface"
git push -u origin praktikum/m10-syscall-abi
```

Output ringkas:

```text
[praktikum/m10-syscall-abi 889411f] M10: implement syscall ABI and INT80 interface
 8 files changed, 663 insertions(+), 7 deletions(-)
...
 * [new branch]      praktikum/m10-syscall-abi -> praktikum/m10-syscall-abi
branch 'praktikum/m10-syscall-abi' set up to track 'origin/praktikum/m10-syscall-abi'.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Commit M10]` | `[889411f]` | `[Snapshot final pekerjaan M10]` |
| `[Branch remote]` | `[origin/praktikum/m10-syscall-abi]` | `[Salinan branch di GitHub untuk pengumpulan/tinjauan]` |

Indikator berhasil:

```text
git commit dan git push berhasil dijalankan tanpa error, branch praktikum/m10-syscall-abi tersedia di remote GitHub (syifanurzimah/MCSOS) dan siap ditinjau atau dipull ulang untuk verifikasi independen.
```

---

## 11. Checkpoint Buildable

Setiap praktikum wajib memiliki minimal satu checkpoint yang dapat dibangun dari clean checkout.

| Checkpoint | Perintah | Expected result | Status |
|---|---|---|---|
| C1 | `` `clang -std=c17 -Iinclude -E kernel/syscall/syscall.c` `` | `[Preprocessing berhasil, header syscall.h ditemukan]` | `[PASS]` |
| C2 | `` `make m10-host-test` `` | `[M10 syscall host tests passed]` | `[PASS setelah perbaikan .RECIPEPREFIX]` |
| C3 | `` `clang --target=x86_64-unknown-none-elf -c kernel/syscall/syscall_entry.S` `` | `[Objek freestanding berhasil dibangun]` | `[PASS]` |
| C4 | `` `make m10-audit` `` | `[nm -u kosong, readelf menunjukkan Machine X86-64, objdump memuat stub dan iretq]` | `[PASS]` |
| C5 | `` `make` (build kernel penuh) `` | `[kernel.elf berhasil dibangun dengan syscall M10 terintegrasi]` | `[PASS setelah perbaikan -Iinclude]` |
| C6 | `` `bash tools/scripts/make_iso.sh` `` | `[build/mcsos.iso berhasil dibuat dengan Limine bios-install]` | `[PASS]` |
| C7 | `` `bash tools/scripts/run_qemu.sh` `` | `[Log serial memuat smoke test syscall M10]` | `[PASS untuk isi log; skrip sendiri tidak mencetak "OK" karena assertion M2 lama tidak relevan]` |
| C8 | `` `git commit` dan `git push` `` | `[Commit 889411f berhasil dibuat dan dipush]` | `[PASS]` |

Catatan checkpoint:

```text
Seluruh checkpoint inti M10 berhasil dilewati: header dan implementasi syscall valid secara sintaks, host unit test lulus, objek freestanding syscall.c dan syscall_entry.S teraudit tanpa simbol undefined, kernel penuh berhasil dibangun setelah include path diperbaiki, dan yang membedakan dari laporan M9 sebelumnya, boot QEMU pada M10 benar-benar berhasil dijalankan dengan log serial yang memuat smoke test syscall. Satu-satunya catatan adalah skrip run_qemu.sh memakai assertion peninggalan M2 yang tidak relevan lagi untuk kernel M10, sehingga output akhir skrip tidak mencetak "OK" walau isi log sudah benar.
```

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

```bash
make clean
make
make m10-host-test
make m10-audit
```

Hasil:

```text
M10 syscall host tests passed
(build/kernel.elf berhasil dibangun, seluruh grep -q assertion bawaan lulus)
```

Status: `[PASS]`

### 12.2 Static Inspection

```bash
nm -u build/m10/m10_syscall_combined.o
readelf -h build/m10/m10_syscall_combined.o
objdump -dr build/m10/m10_syscall_combined.o
nm -n build/kernel.elf | grep -i syscall
objdump -d -Mintel build/kernel.elf
```

Hasil penting:

```text
build/m10/nm_undefined.txt kosong (tidak ada simbol undefined)
ELF64, Machine: Advanced Micro Devices X86-64
Fungsi x86_64_syscall_int80_stub ditemukan pada offset 0x548 (objek gabungan freestanding)
Instruksi iretq ditemukan pada offset 0x589
```

Status: `[PASS]`

### 12.3 QEMU Smoke Test

```bash
bash tools/scripts/make_iso.sh
bash tools/scripts/run_qemu.sh
cat build/qemu-serial.log
```

Hasil:

```text
Boot berhasil lewat Limine. Log serial memuat "[M10] user region initialized", "[M10] syscall subsystem initialized", "[M10] syscall ping ok", "[M10] syscall get_ticks ok", "[M10] syscall smoke done", diikuti "[M9] scheduler initialized" dan tick timer berjalan normal. Skrip run_qemu.sh sendiri tidak mencetak "OK: QEMU serial log valid" karena assertion akhirnya masih memeriksa penanda boot M2 yang sudah tidak dipakai kernel versi M10.
```

Status: `[PASS untuk isi log; NA untuk assertion otomatis skrip lama]`

### 12.4 GDB Debug Evidence

```bash
Belum diterapkan pada M10.
```

Status: `[NA]`

### 12.5 Unit Test

```bash
make m10-host-test
```

Hasil:

```text
./build/m10/test_syscall_host
M10 syscall host tests passed
```

Status: `[PASS]`

### 12.6 Stress/Fuzz/Fault Injection Test

Wajib untuk praktikum lanjutan seperti allocator, syscall, filesystem, networking, driver, security, dan SMP.

```text
Belum diterapkan pada M10. Host unit test yang ada baru mencakup jalur normal dan satu kasus EFAULT (pointer di luar region); belum ada fuzzing terhadap kombinasi nr/argumen acak, belum ada stress test pemanggilan syscall dalam jumlah besar, dan belum ada uji nyata lewat instruksi int $0x80 dari mode pengguna karena gate IDT 0x80 belum diaktifkan pada boot path yang diuji.
```

Status: `[NA]`

### 12.7 Visual Evidence

| Screenshot | Lokasi file | Keterangan |
|---|---|---|
| `[screenshot]` | `[path]` | `[Tidak relevan pada M10, tidak ada output grafis; bukti berupa log serial teks]` |

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| No. | Uji | Expected result | Actual result | Status | Evidence |
|---|---|---|---|---|---|
| 1 | `[Sintaks syscall.h dan syscall.c]` | `[Tidak ada error/warning]` | `[clang -Wall -Wextra -Werror bersih]` | `[PASS]` | `[output terminal Langkah 3–4]` |
| 2 | `[Kompilasi freestanding syscall_entry.S]` | `[Objek berhasil dibangun]` | `[objdump menunjukkan stub dan iretq benar]` | `[PASS]` | `[output terminal Langkah 5]` |
| 3 | `[Host unit test syscall]` | `[Seluruh skenario dispatch lulus]` | `[M10 syscall host tests passed]` | `[PASS]` | `[build/m10/test_syscall_host output]` |
| 4 | `[Audit objek freestanding M10]` | `[ELF64 x86_64, tidak ada undefined symbol]` | `[nm_undefined.txt kosong, readelf/objdump sesuai]` | `[PASS]` | `[build/m10/nm_undefined.txt, readelf_header.txt, objdump.txt]` |
| 5 | `[Build kernel penuh dengan syscall M10]` | `[kernel.elf berhasil dibangun]` | `[Gagal karena -Iinclude belum ada, berhasil setelah diperbaiki]` | `[PASS setelah perbaikan]` | `[build/kernel.elf, build/kernel.map]` |
| 6 | `[Pembuatan image ISO dengan Limine]` | `[build/mcsos.iso valid dan bootable]` | `[Berhasil lewat tools/scripts/make_iso.sh, gagal pada percobaan manual pertama]` | `[PASS]` | `[build/mcsos.iso, build/mcsos.iso.sha256]` |
| 7 | `[Boot QEMU dengan smoke test syscall]` | `[Log memuat "syscall ping ok", "syscall get_ticks ok"]` | `[Log serial sesuai ekspektasi]` | `[PASS]` | `[build/qemu-serial.log]` |
| 8 | `[Assertion otomatis skrip run_qemu.sh]` | `[Mencetak "OK: QEMU serial log valid"]` | `[Tidak tercetak karena assertion memeriksa penanda boot M2 lama]` | `[NA]` | `[keluaran bash tools/scripts/run_qemu.sh]` |
| 9 | `[Pemasangan gate IDT vektor 0x80]` | `[Log "syscall vector 0x80 installed" muncul]` | `[Tidak muncul karena blok kode masih #if 0]` | `[NA]` | `[kernel/core/kmain.c, build/qemu-serial.log]` |
| 10 | `[Commit dan push repository]` | `[Perubahan tersimpan di Git dan remote]` | `[Commit 889411f, branch terpush]` | `[PASS]` | `[git log, git push output]` |

### 13.2 Log Penting

```text
M10 syscall host tests passed

[M10] user region initialized
[M10] syscall subsystem initialized
[M10] syscall ping ok
[M10] syscall get_ticks ok
[M10] syscall smoke done
```

---

## 14. Analisis Teknis

```text
Dispatcher syscall berbasis tabel fungsi (g_table) terbukti mempermudah penambahan syscall baru: menambah entri enum mcsos_syscall_nr_t, menulis fungsi sys_*, dan menambahkan pointer fungsi pada g_table sudah cukup tanpa mengubah logika mcsos_syscall_dispatch. Validasi rentang pointer (mcsos_user_check_range) yang secara eksplisit memeriksa potensi overflow (addr+len) merupakan bagian paling kritis dari sisi keamanan, karena kesalahan di sini dapat membuka akses baca/tulis kernel ke alamat yang tidak diinginkan. Pemisahan antara jalur pemanggilan langsung (mcsos_syscall_dispatch dipanggil dari C) dan jalur INT 0x80 (mcsos_syscall_dispatch_frame dipanggil dari assembly) terbukti berguna untuk pengujian bertahap: jalur langsung dapat diuji sepenuhnya lewat host unit test dan smoke test kmain() tanpa perlu menunggu integrasi IDT selesai, sementara jalur INT 0x80 dapat diverifikasi struktur instruksinya lewat objdump sebelum benar-benar diaktifkan pada boot path. Kelemahan utama pada implementasi saat ini adalah dispatcher dan region user bersifat global tanpa proteksi konkurensi, serta gate IDT 0x80 belum benar-benar aktif sehingga klaim "syscall ABI siap dipakai dari mode pengguna" belum sepenuhnya terbukti pada runtime.
```

---

## 15. Debugging dan Failure Modes

### 15.1 Kegagalan Makefile "missing separator" Berulang

```text
Root cause: Makefile proyek ini memakai ".RECIPEPREFIX := >" (karakter '>' sebagai pengganti tab standar Make) yang telah ditetapkan sejak milestone sebelumnya. Saat target M10 ditambahkan lewat heredoc (cat >> Makefile) dan diedit ulang lewat nano, sebagian baris resep baru tertulis dengan karakter tab, bukan '>', sehingga make menolaknya dengan "missing separator" — dan karena baris pemicu berada sebelum banyak resep lama pada file, seluruh resep lama (M6/M8/M9) ikut gagal dikenali begitu baris .RECIPEPREFIX sempat hilang/tidak sinkron.
Diagnosis: menggunakan "cat -A"/"cat -te", "nl -ba", dan "od -An -tx1" untuk melihat karakter literal pada baris yang dicurigai, membandingkan resep baru dengan resep lama yang diketahui berfungsi.
Perbaikan: menyisipkan kembali baris ".RECIPEPREFIX := >" tepat setelah baris SHELL (baris ke-2) dengan "sed -i '2i .RECIPEPREFIX := >' Makefile", kemudian memastikan tidak ada baris resep yang tersisa berawalan tab murni dengan "grep -nP '^\t' Makefile" (harus kosong).
```

### 15.2 Header `mcsos/syscall.h` Tidak Ditemukan saat Build Freestanding Penuh

```text
Root cause: COMMON_CFLAGS dan COMMON_ASFLAGS pada Makefile hanya memuat -Ikernel/arch/x86_64/include dan -Ikernel/include, belum memuat -Iinclude, padahal include/mcsos/syscall.h ditulis di direktori include/ pada root repository (konsisten dengan pola M9 yang menaruh mcsos_thread.h di include/mcsos/ lalu menyalinnya ke kernel/include/mcsos/).
Diagnosis: kompilasi mandiri "clang -Iinclude -E kernel/syscall/syscall.c" berhasil, tetapi "make" penuh gagal dengan "fatal error: 'mcsos/syscall.h' file not found", menunjukkan perbedaan flag antara kompilasi manual dan target Makefile.
Perbaikan: menambahkan " -Iinclude" ke akhir baris COMMON_CFLAGS dan COMMON_ASFLAGS dengan sed; satu percobaan sed sempat menghasilkan baris duplikat/tidak menyatu (" -Iinclude" berdiri sendiri sebagai baris baru) akibat penomoran baris yang bergeser antar dua pemanggilan sed berurutan, yang kemudian diperbaiki dengan menghapus baris duplikat tersebut ("sed -i '24d' Makefile").
```

### 15.3 ISO Pertama Gagal Boot (Serial Log Kosong)

```text
Root cause: percobaan pertama membangun build/mcsos.iso secara manual langsung dengan xorriso (tanpa menjalankan tahap "limine bios-install" pada ISO yang dihasilkan) menghasilkan image yang tidak benar-benar bisa di-boot BIOS, sehingga QEMU langsung keluar tanpa menuliskan apa pun ke serial log.
Diagnosis: bash tools/scripts/run_qemu.sh melaporkan "ERROR: serial log kosong: build/qemu-serial.log" tepat setelah QEMU berhenti karena timeout, menandakan tidak ada satu pun byte yang ditulis kernel/limine ke port serial.
Perbaikan: menjalankan skrip resmi tools/scripts/make_iso.sh yang menyalin kernel.elf, limine.conf, dan berkas Limine BIOS/UEFI ke iso_root/, membangun ISO dengan opsi xorriso yang lengkap (termasuk --efi-boot-image dan --protective-msdos-label), lalu menjalankan "$LIMINE_DIR/limine" bios-install "$ISO" agar stage 2 Limine benar-benar tertanam pada image; setelah ini QEMU berhasil boot dan serial log terisi penuh.
```

### 15.4 Panic Path

```text
Panic path M10 memakai makro KERNEL_PANIC yang sudah tersedia sejak milestone sebelumnya (kernel_panic_at, dipanggil lewat KERNEL_PANIC(reason, code)). Pada M10, panic dipicu bila m10_syscall_smoke_direct() menerima nilai balik yang tidak sesuai: KERNEL_PANIC("M10 syscall ping failed", r) bila MCSOS_SYS_PING tidak mengembalikan 0x2605020A, dan KERNEL_PANIC("M10 syscall get_ticks failed", r) bila MCSOS_SYS_GET_TICKS mengembalikan nilai negatif. Pada sesi ini panic tersebut tidak pernah terpicu karena kedua syscall berhasil dengan nilai yang sesuai (dibuktikan lewat log "[M10] syscall ping ok" dan "[M10] syscall get_ticks ok"), sehingga jalur panic belum teruji secara aktual pada runtime M10, hanya tersedia sebagai mekanisme defensif.
```

### 15.5 Ringkasan Failure Mode

| Kegagalan | Gejala | Root cause | Perbaikan | Status |
|---|---|---|---|---|
| `[Makefile missing separator]` | `[make m10-host-test/make gagal total]` | `[.RECIPEPREFIX := > hilang/tidak sinkron akibat heredoc dan nano]` | `[Menyisipkan kembali .RECIPEPREFIX := > dan menormalkan seluruh baris resep]` | `[Selesai]` |
| `[Header syscall.h tidak ditemukan]` | `[fatal error: file not found saat build penuh]` | `[COMMON_CFLAGS/COMMON_ASFLAGS belum memuat -Iinclude]` | `[Menambahkan -Iinclude, membersihkan baris duplikat hasil sed]` | `[Selesai]` |
| `[ISO pertama tidak bisa boot]` | `[Serial log kosong, QEMU langsung keluar]` | `[ISO dibangun manual tanpa limine bios-install]` | `[Memakai skrip resmi make_iso.sh]` | `[Selesai]` |
| `[Assertion run_qemu.sh tidak lulus]` | `[Skrip tidak mencetak "OK" walau log benar]` | `[Assertion memeriksa penanda boot M2 yang sudah tidak relevan untuk kernel M10]` | `[Belum diperbaiki; dicatat sebagai known issue]` | `[Belum selesai]` |
| `[Gate IDT 0x80 tidak aktif]` | `[Log "syscall vector 0x80 installed" tidak muncul]` | `[Blok pemasangan gate sengaja dibungkus #if 0 sebagai keputusan cakupan]` | `[Belum diaktifkan; direncanakan pada milestone lanjutan]` | `[Belum selesai]` |

---

## 16. Prosedur Rollback

```text
Repository tidak memerlukan rollback berat pada sesi M10 ini (berbeda dengan M9 yang sempat memakai "git reset --hard"), karena kesalahan yang terjadi (Makefile missing separator, header tidak ditemukan, ISO gagal boot) seluruhnya bersifat dapat diperbaiki di tempat (in place) tanpa merusak file sumber C/assembly yang sudah benar. Prosedur rollback yang tersedia dan disiapkan sebagai jaring pengaman:
1. "git status --short" dan "git diff" dijalankan sebelum setiap commit untuk memastikan hanya perubahan yang dimaksud yang disertakan.
2. Salinan cadangan manual dibuat sebelum edit besar: "cp Makefile Makefile.bak" sebelum menormalkan .RECIPEPREFIX, dan "cp include/mcsos/mcsos_thread.h include/mcsos/mcsos_thread.h.bak" (peninggalan sesi M9) tetap tersimpan sebagai referensi.
3. Bila commit 889411f perlu dibatalkan, "git revert 889411f" atau "git reset --hard 4453b81" (commit akhir M9) dapat dipakai untuk kembali ke kondisi sebelum M10, karena branch praktikum/m10-syscall-abi terisolasi dari branch m9-kernel-thread-scheduler.
Prosedur "git reset --hard" maupun "git revert" belum benar-benar dieksekusi pada sesi M10 ini karena tidak diperlukan; keduanya dicatat sebagai prosedur yang tersedia, bukan yang telah diuji pada milestone ini.
```

---

## 17. Keamanan dan Reliability

### 17.1 Ringkasan Keamanan

```text
Keamanan utama M10 terletak pada validasi pointer user sebelum dipakai kernel: mcsos_user_check_range menolak alamat di luar [g_user_region.base, g_user_region.limit), menolak potensi integer overflow pada addr+len, dan mcsos_copy_from_user menegakkan pemeriksaan ini sebelum menyalin byte apa pun. Nomor syscall di luar rentang (nr >= MCSOS_SYS_MAX) selalu ditolak dengan MCSOS_ENOSYS sebelum menyentuh tabel fungsi, mencegah pemanggilan fungsi lewat indeks di luar batas. Namun demikian, boundary keamanan yang paling penting secara konsep — titik masuk dari mode pengguna tidak tepercaya lewat instruksi int $0x80 — belum benar-benar aktif pada boot path yang diuji (gate IDT 0x80 masih dalam blok #if 0), sehingga seluruh pengujian keamanan yang ada baru mencakup pemanggilan dari kode kernel yang sudah tepercaya (m10_syscall_smoke_direct dan host unit test), bukan dari kode mode pengguna sungguhan.
```

### 17.2 Tabel Ancaman

| Ancaman | Vektor | Mitigasi saat ini | Kecukupan |
|---|---|---|---|
| `[Pointer user tidak valid pada MCSOS_SYS_WRITE_SERIAL]` | `[Argumen ptr/len dari pemanggil]` | `[mcsos_user_check_range + batas len<=4096]` | `[Cukup untuk kasus yang diuji host test; belum diuji lewat int $0x80 nyata]` |
| `[Integer overflow pada validasi rentang]` | `[addr+len yang di-craft agar wraparound]` | `[Pemeriksaan eksplisit "last < addr"]` | `[Cukup, diuji secara implisit lewat logika, belum ada test host khusus untuk kasus overflow]` |
| `[Nomor syscall di luar rentang atau negatif (setelah cast ke uint64_t akan menjadi besar)]` | `[Nilai nr sembarang]` | `[Perbandingan nr >= MCSOS_SYS_MAX sebelum indexing g_table]` | `[Cukup, diuji host test dengan nr=999]` |
| `[Kode mode pengguna tidak tepercaya memanggil int $0x80]` | `[Instruksi int $0x80 dari ring3]` | `[Belum ada — gate IDT 0x80 belum diaktifkan pada boot path]` | `[Belum cukup, ini known issue utama M10]` |
| `[Konkurensi pemanggilan syscall dari banyak thread/CPU]` | `[g_ops/g_user_region diakses tanpa lock]` | `[Belum ada mekanisme locking]` | `[Belum cukup untuk skenario preemptive/SMP]` |

### 17.3 Reliability

```text
Reliability level host (unit test) dan level build (audit ELF, assertion Makefile) sudah baik: seluruh target m10-host-test dan m10-audit lulus secara konsisten dan dapat diulang dari clean checkout ("make m10-clean" lalu target terkait). Reliability level runtime juga terbukti untuk jalur pemanggilan langsung lewat log serial QEMU nyata. Yang belum terbukti reliable adalah jalur INT 0x80 pada kondisi nyata (belum diaktifkan) dan perilaku sistem saat syscall dipanggil bersamaan dari lebih dari satu thread setelah scheduler M9 benar-benar melakukan preemption.
```

---

## 18. Pembagian Kerja Kelompok

Isi bagian ini hanya jika praktikum dikerjakan berkelompok. Untuk pengerjaan individu, tulis "Tidak berlaku".

```text
Tidak berlaku. Praktikum M10 dikerjakan secara individu oleh Syifa Nurzimah (NIM 25832074009).
```

### 18.1 Mekanisme Koordinasi

```text
Tidak berlaku untuk pengerjaan individu.
```

### 18.2 Evaluasi Kontribusi

| Anggota | Persentase kontribusi yang disepakati | Bukti | Catatan |
|---|---:|---|---|
| `[Syifa Nurzimah]` | `[100%]` | `[commit 889411f]` | `[Pengerjaan individu]` |

---

## 19. Kriteria Lulus Praktikum

Bagian ini wajib diisi. Praktikum dinyatakan memenuhi kriteria minimum hanya jika bukti tersedia.

| Kriteria minimum | Status | Evidence |
|---|---|---|
| Proyek dapat dibangun dari clean checkout | `[PASS]` | `[make clean && make, make m10-host-test, make m10-audit]` |
| Perintah build terdokumentasi | `[PASS]` | `[Bagian 10 Langkah Kerja Implementasi]` |
| QEMU boot atau test target berjalan deterministik | `[PASS]` | `[build/qemu-serial.log memuat smoke test syscall]` |
| Semua unit test/praktikum test relevan lulus | `[PASS]` | `[M10 syscall host tests passed]` |
| Log serial disimpan | `[PASS]` | `[build/qemu-serial.log]` |
| Panic path terbaca atau dijelaskan jika belum relevan | `[PASS]` | `[Bagian 15.4 Panic Path]` |
| Tidak ada warning kritis pada build | `[PASS]` | `[-Wall -Wextra -Werror bersih pada seluruh langkah setelah perbaikan]` |
| Perubahan Git terkomit | `[PASS]` | `[commit 889411f]` |
| Desain dan failure mode dijelaskan | `[PASS]` | `[Bagian 9 Desain Teknis dan 15 Failure Modes]` |
| Laporan berisi screenshot/log yang cukup | `[PASS]` | `[Lampiran evidence terminal]` |

Kriteria tambahan untuk praktikum lanjutan:

| Kriteria lanjutan | Status | Evidence |
|---|---|---|
| Static analysis dijalankan | `[PASS]` | `[clang -Wall -Wextra -Werror pada seluruh file M10]` |
| Stress test dijalankan | `[NA]` | `[belum diterapkan pada M10]` |
| Fuzzing atau malformed-input test dijalankan | `[NA]` | `[belum diterapkan pada M10, hanya satu kasus EFAULT manual]` |
| Fault injection dijalankan | `[NA]` | `[belum diterapkan pada M10]` |
| Disassembly/readelf evidence tersedia | `[PASS]` | `[build/m10/objdump.txt, build/m10/readelf_header.txt, build/kernel.disasm.txt]` |
| Review keamanan dilakukan | `[PASS]` | `[Bagian 17 Keamanan dan Reliability]` |
| Rollback diuji | `[NA]` | `[Prosedur tersedia (Bagian 16) tetapi belum benar-benar dieksekusi pada M10]` |

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
Seluruh tahapan build dan test level host untuk M10 berhasil dijalankan: sintaks syscall.h dan syscall.c valid, host unit test menunjukkan "M10 syscall host tests passed", audit objek freestanding syscall dan stub INT 0x80 menunjukkan struktur ELF64 x86_64 yang benar tanpa simbol undefined, kernel penuh berhasil dibangun setelah include path diperbaiki, dan berbeda dari laporan M9 sebelumnya, image ISO berhasil dibuat dengan Limine dan boot QEMU benar-benar berhasil dijalankan dengan log serial yang memuat smoke test syscall ("syscall ping ok", "syscall get_ticks ok"). Namun demikian, gate IDT vektor 0x80 (titik masuk INT 0x80 yang menjadi inti judul milestone ini) masih dinonaktifkan lewat blok #if 0 pada kmain.c dan belum pernah diuji lewat instruksi int $0x80 yang sesungguhnya dari kode yang meniru mode pengguna, sehingga status readiness ditetapkan "Siap uji QEMU" (build dan smoke test jalur langsung sudah terbukti, tinggal mengaktifkan dan menguji jalur INT 0x80 secara nyata) dan bukan "Siap demonstrasi praktikum".
```

Known issues:

| No. | Issue | Dampak | Workaround | Target perbaikan |
|---|---|---|---|---|
| 1 | `[Gate IDT vektor 0x80 belum diaktifkan (dibungkus #if 0 pada kmain.c)]` | `[Jalur syscall lewat int $0x80 dari mode pengguna belum pernah teruji end-to-end]` | `[Verifikasi lewat pemanggilan langsung mcsos_syscall_dispatch dan audit statis objdump pada stub]` | `[Mengaktifkan blok x86_64_idt_set_gate(0x80, ...) dan menguji dengan kode yang mengeksekusi int $0x80]` |
| 2 | `[Skrip tools/scripts/run_qemu.sh memakai assertion penanda boot M2 yang sudah tidak relevan untuk kernel M10]` | `[Skrip tidak mencetak "OK: QEMU serial log valid" walau boot dan smoke test sebenarnya berhasil]` | `[Verifikasi manual isi build/qemu-serial.log]` | `[Memperbarui assertion skrip agar sesuai penanda log milestone terbaru (M8–M10)]` |
| 3 | `[Belum ada mekanisme locking pada g_ops/g_user_region untuk konteks interrupt/SMP]` | `[Berpotensi race condition bila syscall dipanggil bersamaan dari beberapa thread setelah preemption diaktifkan]` | `[Scheduler M9 masih kooperatif sehingga risiko saat ini rendah]` | `[Menambahkan locking dasar saat preemption/SMP diimplementasikan]` |
| 4 | `[File tests/test_syscall.c dan tests/test_syscall_host.c serupa; hanya yang kedua dipakai Makefile]` | `[Berpotensi membingungkan kontributor lain]` | `[Dokumentasikan pada laporan bahwa test_syscall_host.c adalah versi resmi]` | `[Menghapus atau menggabungkan tests/test_syscall.c pada milestone berikutnya]` |
| 5 | `[File sampah untracked (M10.txt, Makefile.bak, mcsos_thread.h.bak, logs/) belum dibersihkan/di-commit]` | `[Repository tidak sepenuhnya rapi]` | `[git status ditinjau manual sebelum commit berikutnya]` | `[Menjalankan git clean -fd atau memindahkan berkas ke luar repository setelah verifikasi]` |
| 6 | `[Belum ada stress/fuzz test untuk kombinasi nr dan argumen acak]` | `[Klaim robustness syscall terhadap input tak terduga belum sepenuhnya diverifikasi]` | `[Validasi manual pada satu kasus EFAULT dan satu kasus ENOSYS]` | `[Menambahkan fuzz test pada milestone lanjutan (M12 Security)]` |

Keputusan akhir:

```text
Berdasarkan hasil make m10-host-test, make m10-audit, build kernel penuh, dan boot QEMU dengan log serial yang seluruhnya berhasil dijalankan, praktikum M10 dinyatakan siap uji QEMU sebagai dasar untuk mengaktifkan dan menguji jalur INT 0x80 secara nyata pada milestone berikutnya.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Indikator nilai penuh | Nilai |
|---|---:|---|---:|
| Kebenaran fungsional | 30 | Implementasi memenuhi target praktikum, build/test lulus, output sesuai expected result | `[27]` |
| Kualitas desain dan invariants | 20 | Desain jelas, kontrak antarmuka eksplisit, invariants/ownership/locking terdokumentasi | `[17]` |
| Pengujian dan bukti | 20 | Unit/integration/QEMU/static/fuzz/stress evidence memadai sesuai tingkat praktikum | `[16]` |
| Debugging dan failure analysis | 10 | Failure mode, triage, panic/log, dan rollback dianalisis | `[9]` |
| Keamanan dan robustness | 10 | Boundary, input validation, privilege, memory safety, dan negative tests dibahas | `[7]` |
| Dokumentasi dan laporan | 10 | Laporan rapi, lengkap, dapat direproduksi, memakai referensi yang layak | `[9]` |
| **Total** | **100** |  | `[85]` |

Catatan penilai:

```text
[Diisi dosen/asisten.]
```

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Kontrak ABI syscall MCSOS (mcsos_syscall_frame_t, mcsos_syscall_nr_t, mcsos_syscall_status_t, mcsos_user_region_t, mcsos_syscall_ops_t) berhasil didesain dan diimplementasikan dalam C17 freestanding pada include/mcsos/syscall.h dan kernel/syscall/syscall.c, lengkap dengan dispatcher berbasis tabel fungsi, validasi rentang pointer user yang menangani kasus overflow, dan lima syscall dasar (ping, get_ticks, write_serial, yield, exit_thread). Stub assembly x86_64_syscall_int80_stub berhasil ditulis dan diverifikasi lewat objdump menunjukkan urutan penyimpanan register dan instruksi iretq yang benar. Host unit test lulus untuk seluruh skenario dispatch, copy_from_user, dan kode error. Setelah memperbaiki dua kelas kegagalan Makefile (missing separator akibat .RECIPEPREFIX, dan include path -Iinclude yang belum ada), kernel penuh berhasil dibangun dengan subsistem syscall terintegrasi, image ISO berhasil dibuat memakai Limine, dan boot QEMU nyata berhasil dijalankan dengan log serial yang membuktikan smoke test syscall ("[M10] syscall ping ok", "[M10] syscall get_ticks ok", "[M10] syscall smoke done") berjalan sebelum scheduler M9 diinisialisasi. Seluruh perubahan telah dikomit (889411f) dan dipush ke repository pada branch praktikum/m10-syscall-abi.
```

### 22.2 Yang Belum Berhasil

```text
Gate IDT vektor 0x80 yang menjadi inti antarmuka INT 0x80 pada judul milestone ini belum benar-benar diaktifkan pada boot path yang diuji (kode pemasangannya masih dibungkus blok #if 0 pada kmain.c), sehingga jalur syscall lewat instruksi int $0x80 dari kode yang meniru mode pengguna belum pernah diuji secara end-to-end pada runtime nyata; verifikasi yang tersedia baru berupa audit statis disassembly stub. Skrip tools/scripts/run_qemu.sh masih memakai assertion penanda boot M2 lama yang tidak relevan untuk kernel M10 sehingga tidak mencetak status "OK" walau isi log sebenarnya benar. Belum ada mekanisme locking untuk g_ops/g_user_region, belum ada stress/fuzz test untuk kombinasi argumen syscall acak, dan masih ada berkas duplikat (tests/test_syscall.c) serta beberapa file sampah untracked (M10.txt, Makefile.bak, mcsos_thread.h.bak, logs/) yang belum dirapikan.
```

### 22.3 Rencana Perbaikan

```text
Mengaktifkan pemasangan gate IDT vektor 0x80 pada kmain.c (menghapus blok #if 0) dan menguji jalur INT 0x80 secara nyata memakai kode yang mengeksekusi instruksi int $0x80 langsung, memperbarui assertion pada tools/scripts/run_qemu.sh agar sesuai dengan penanda log milestone terkini, menambahkan locking dasar pada g_ops/g_user_region sebagai persiapan preemption/SMP, menambahkan stress test dan fuzz test untuk kombinasi nomor syscall dan argumen acak (termasuk pengujian eksplisit kasus overflow addr+len), menghapus atau menggabungkan tests/test_syscall.c dengan tests/test_syscall_host.c, serta membersihkan file sampah untracked pada repository sebagai persiapan menuju milestone berikutnya.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
889411f M10: implement syscall ABI and INT80 interface
4453b81 M9: implement cooperative kernel scheduler
a44e07b checkpoint before M9 scheduler
03657c2 M8 kernel heap implementation
```

### Lampiran B — Diff Ringkas

```diff
Perubahan utama:
- Menambahkan include/mcsos/syscall.h (kontrak ABI syscall lengkap, 68 baris)
- Menambahkan kernel/syscall/syscall.c (dispatcher syscall, 112 baris)
- Menambahkan kernel/syscall/syscall_entry.S (stub INT 0x80, 27 baris)
- Menambahkan tests/test_syscall_host.c (host unit test syscall, 106 baris) dan tests/test_syscall.c (draf awal, tidak dipakai Makefile)
- Menambahkan target M10_BUILD, m10-all, m10-host-test, m10-audit, m10-clean pada Makefile
- Memperbaiki .RECIPEPREFIX := > yang sempat hilang pada Makefile
- Menambahkan -Iinclude pada COMMON_CFLAGS dan COMMON_ASFLAGS Makefile
- Mengubah kernel/core/kmain.c: menambahkan #include syscall.h, extern x86_64_syscall_int80_stub, k_get_ticks/k_yield_current/k_exit_current/k_write_serial, MCSOS_USER_BASE/LIMIT, mcsos_syscall_init, mcsos_syscall_set_user_region, dan m10_syscall_smoke_direct()
- Membuat build/mcsos.iso lewat tools/scripts/make_iso.sh (Limine + xorriso)
```

### Lampiran C — Log Build Lengkap

```text
M10 syscall host tests passed
build/m10/nm_undefined.txt kosong (tidak ada simbol undefined)
readelf -h build/m10/m10_syscall_combined.o: ELF64, Machine Advanced Micro Devices X86-64
x86_64_syscall_int80_stub ditemukan pada offset 0x548, iretq pada offset 0x589
kernel.elf berhasil dibangun dengan seluruh assertion audit bawaan (ELF64, Machine X86-64, kmain, x86_64_idt_init, x86_64_trap_dispatch, iretq, lidt) lulus
```

### Lampiran D — Log QEMU Lengkap

```text
limine: Loading executable `boot():/boot/kernel.elf`...
[MCSOS:M5] boot: external interrupt bring-up start
idt_base=0xffffffff80007000
idt_limit=0x0000000000000fff
[M4] IDT loaded
[MCSOS:M5] idt: loaded
[MCSOS:M5] pic: remapped; mask master=0x00000000000000fe slave=0x00000000000000ff
[MCSOS:M5] pit: configured 100Hz
[M8] kernel heap initialized
[M10] user region initialized
[M10] syscall subsystem initialized
[M10] syscall ping ok
[M10] syscall get_ticks ok
[M10] syscall smoke done
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
readelf -h build/m10/m10_syscall_combined.o : ELF64, Type REL, Machine Advanced Micro Devices X86-64
objdump -dr build/m10/m10_syscall_combined.o : x86_64_syscall_int80_stub pada offset 0x548, iretq pada offset 0x589
nm -u build/m10/m10_syscall_combined.o : kosong (tidak ada simbol undefined)
sha256sum build/mcsos.iso : e2e073df62edbb1b72a451452e3bd7d3ff15e4ba985a0c23a0f6b7c4047e47e6
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[path/screenshot]` | `[belum dilampirkan pada transkrip yang tersedia]` |

### Lampiran G — Bukti Tambahan

```text
build/m10/test_syscall_host
build/m10/syscall.o
build/m10/syscall_entry.o
build/m10/m10_syscall_combined.o
build/m10/nm_undefined.txt
build/m10/readelf_header.txt
build/m10/objdump.txt
build/mcsos.iso
build/mcsos.iso.sha256
build/qemu-serial.log
build/kernel.elf
build/kernel.syms.txt
build/kernel.disasm.txt
build/kernel.map
logs/m10_sha256.txt
```

---

## 24. Daftar Referensi

Gunakan format IEEE. Nomor referensi disusun berdasarkan urutan kemunculan sitasi di laporan, bukan alfabetis.

Referensi yang benar-benar dipakai dalam laporan:

```text
[1] R. H. Arpaci-Dusseau and A. C. Arpaci-Dusseau, Operating Systems: Three Easy Pieces, bab Mechanism: Limited Direct Execution. [Online]. Available: [URL]. Accessed: [tanggal akses].
[2] Intel Corporation, Intel 64 and IA-32 Architectures Software Developer's Manual, bagian Interrupt/Exception Handling (IDT gate, IRET). [Online]. Available: [URL]. Accessed: [tanggal akses].
[3] GNU Binutils Documentation (nm, readelf, objdump). [Online]. Available: [URL]. Accessed: [tanggal akses].
[4] LLVM/Clang Documentation. [Online]. Available: [URL]. Accessed: [tanggal akses].
[5] Limine Bootloader Documentation dan xorriso (RockRidge/ISO9660) Documentation. [Online]. Available: [URL]. Accessed: [tanggal akses].
[6] Git Documentation (git-commit, git-push, git-branch). [Online]. Available: [URL]. Accessed: [tanggal akses].
```

---

## 25. Checklist Final Sebelum Pengumpulan

| Checklist | Status |
|---|---|
| Semua placeholder `[isi ...]` sudah diganti | `[Tidak — beberapa field seperti tanggal pengumpulan dan hash SHA-256 sebagian artefak masih perlu diverifikasi ulang mahasiswa]` |
| Metadata laporan lengkap | `[Sebagian, lihat catatan di atas]` |
| Commit awal dan akhir dicatat | `[Ya — commit awal 4453b81, commit akhir 889411f]` |
| Perintah build dan test dapat dijalankan ulang | `[Ya]` |
| Log build dilampirkan | `[Ya]` |
| Log QEMU/test dilampirkan | `[Ya]` |
| Artefak penting diberi hash | `[Ya — build/mcsos.iso.sha256, logs/m10_sha256.txt]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya, sebagai penilaian mandiri]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya/kami mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
889411f
```

Status akhir yang diklaim:

```text
Siap uji QEMU
```

Ringkasan satu paragraf:

```text
Praktikum M10 berhasil menghasilkan kontrak ABI syscall MCSOS (mcsos syscall) lengkap dengan dispatcher berbasis tabel fungsi, validasi rentang pointer user, dan stub assembly INT 0x80, teruji melalui host unit test dan audit objek freestanding tanpa simbol undefined. Setelah memperbaiki dua kelas kegagalan Makefile (RECIPEPREFIX yang hilang dan include path yang belum lengkap), kernel penuh berhasil dibangun, image ISO berhasil dibuat memakai Limine, dan boot QEMU nyata berhasil dijalankan dengan log serial yang membuktikan smoke test dua syscall (ping dan get_ticks) berjalan sebelum scheduler M9 diinisialisasi. Keterbatasan utama adalah gate IDT vektor 0x80 belum benar-benar diaktifkan pada boot path yang diuji sehingga jalur syscall dari mode pengguna sesungguhnya lewat instruksi int $0x80 belum diverifikasi end-to-end, skrip run_qemu.sh masih memakai assertion penanda boot M2 lama yang tidak relevan, dan belum ada locking maupun stress/fuzz test untuk syscall. Langkah berikutnya adalah mengaktifkan gate IDT 0x80, menguji jalur int $0x80 secara nyata, memperbarui assertion skrip QEMU, dan menambahkan pengujian keamanan yang lebih menyeluruh pada milestone selanjutnya.
```
