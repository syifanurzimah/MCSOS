 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M0

**Nama file laporan:** `laporan_praktikum_m4_25832074009.md`  
**Nama sistem operasi:** MCSOS versi 260502  
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset  
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.  
**Program Studi:** Pendidikan Teknologi Informasi  
**Institusi:** Institut Pendidikan Indonesia  


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M4]` |
| Judul praktikum | `[IDT, Exception, Trap Dispatch, dan Debugging Lanjut]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimmah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[isi jika kelompok]` |
| Anggota kelompok | `[nama, NIM, peran ringkas]` |
| Tanggal praktikum | `[2026-06-30]` |
| Tanggal pengumpulan | `[YYYY-MM-DD]` |
| Repository | `[https://github.com/syifanurzimah/MCSOS]` |
| Branch | `[m4-idt-exception-path]` |
| Commit awal | `` `[06de7f8]` `` |
| Commit akhir | `` `[82fccdf]` `` |
| Status readiness yang diklaim | `[Siap uji QEMU]` |

---

## 1. Sampul

# Laporan Praktikum `M4`  
## `IDT, Exception, Trap Dispatch, dan Debugging Lanjut`

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
Clang, QEMU, GDB, Limine Bootloader, serta AI Assistant (ChatGPT)
untuk membantu menjelaskan konsep, memperbaiki error build,
menyusun script pendukung, dan membantu penyusunan laporan.

Seluruh kode yang dihasilkan diverifikasi kembali dengan proses build,
audit ELF, pengujian pada QEMU, debugging menggunakan GDB,
serta grading lokal dengan hasil SCORE=90/100.
```

---

## 3. Tujuan Praktikum

Tuliskan tujuan teknis dan konseptual praktikum. Tujuan harus dapat diuji.

1. `Membangun kernel MCSOS milestone M4 yang memiliki Interrupt Descriptor Table (IDT) dan jalur dispatch exception.`
2. `Mengimplementasikan stub interrupt service routine (ISR) dalam assembly serta trap dispatcher dalam C untuk menangani exception CPU.`
3. `Melakukan pengujian exception breakpoint (#BP) secara terkontrol dan memverifikasi jalur panic pada exception lain.`
4. `Memvalidasi kernel menggunakan audit ELF, disassembly, QEMU smoke test, debugging GDB, dan pengumpulan evidence.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Mampu membangun kernel M4 dengan IDT dan trap dispatch]` | `[build berhasil, kernel.elf, kernel.map]` |
| `[Mampu melakukan audit struktur ELF, symbol IDT, dan disassembly LIDT/IRETQ]` | `[readelf, objdump, nm, m4_audit_elf.sh]` |
| `[Mampu melakukan debugging exception kernel menggunakan GDB]` | `[Breakpoint kmain, x86_64_idt_init, x86_64_trap_dispatch, register]` |

---

## 5. Peta Milestone MCSOS

Centang milestone yang menjadi fokus laporan ini. Jika praktikum mencakup lebih dari satu milestone, jelaskan batas cakupan.

| Milestone | Fokus | Status dalam laporan |
|---|---|---|
| M0 | Requirements, governance, baseline arsitektur | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M1 | Toolchain reproducible, Git, QEMU, GDB, metadata build | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M2 | Boot image, kernel ELF64, early console | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M3 | Panic path, linker map, GDB, observability awal | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M4 | Trap, exception, interrupt, timer | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M5 | PMM, VMM, page table, kernel heap | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M6 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M7 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M8 | VFS, file descriptor, ramfs | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M9 | Block layer dan device model | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M10 | Persistent filesystem, mcsfs/ext2-like, recovery | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M11 | Networking stack, packet parsing, UDP/TCP subset | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M12 | Security model, capability/ACL, syscall fuzzing, hardening | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M13 | SMP, scalability, lock stress, NUMA-aware preparation | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M14 | Framebuffer, graphics console, visual regression | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M15 | Virtualization/container subset | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Batas cakupan praktikum:

```text
Praktikum M4 berfokus pada implementasi Interrupt Descriptor Table (IDT),
stub interrupt service routine (ISR) dalam assembly, trap dispatcher
dalam C, pengujian exception breakpoint (#BP) secara terkontrol,
audit struktur ELF, debugging menggunakan GDB, serta pengumpulan
evidence hasil pengujian.

Praktikum ini belum membahas timer interrupt (PIT/APIC), manajemen
memori, scheduler, maupun subsystem lain yang akan dikembangkan
pada milestone berikutnya.
```

---

## 6. Dasar Teori Ringkas

Tuliskan teori yang langsung diperlukan untuk memahami praktikum. Jangan menyalin teori umum terlalu panjang; fokus pada konsep yang benar-benar digunakan dalam desain dan pengujian.

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Pada praktikum M4, konsep utama yang dipelajari adalah mekanisme interrupt dan exception pada arsitektur x86_64, yaitu bagaimana CPU mengalihkan eksekusi ke handler tertentu ketika terjadi exception (misalnya breakpoint #BP). Kernel harus menyiapkan Interrupt Descriptor Table (IDT), memuatnya menggunakan instruksi LIDT, menyediakan stub ISR untuk setiap vector exception, serta melakukan dispatch ke fungsi C yang menangani trap dan mencetak informasi register sebelum kembali (IRETQ) atau melakukan panic.
```

### 6.2 Konsep Arsitektur x86_64 yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[Interrupt Descriptor Table (IDT)]` | `[Tabel 256 entry yang memetakan vector exception/interrupt ke handler]` | `[x86_64_idt_init, idt_base, idt_limit]` |
| `[Instruksi LIDT]` | `[Memuat alamat dan limit IDT ke register CPU]` | `[objdump grep 'lidt']` |
| `[Instruksi IRETQ]` | `[Mengembalikan eksekusi dari interrupt/exception handler]` | `[objdump grep 'iretq']` |
| `[ISR Stub (assembly)]` | `[Menangkap context CPU sebelum memanggil trap dispatcher C]` | `[isr.S, isr_stub_14]` |
| `[GDB breakpoint]` | `[Menghentikan eksekusi di kmain(), x86_64_idt_init(), dan x86_64_trap_dispatch()]` | `[breakpoint berhasil aktif]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding, ditambah assembly x86_64 untuk ISR stub]` |
| Runtime | `[tanpa hosted libc]` |
| ABI | `[x86_64 System V ABI]` |
| Risiko undefined behavior | `[Layout IDT entry yang salah, stack alignment saat masuk/keluar interrupt, urutan push/pop register yang tidak konsisten, dan kesalahan limit/base saat LIDT]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[QEMU documentation]` | `[GDB Stub (-s -S)]` | `[Debugging kernel exception]` |
| `[2]` | `[ELF Specification]` | `[ELF Header & Symbol Table]` | `[Analisis executable kernel dan symbol IDT/trap]` |
| `[3]` | `[GNU GDB Documentation]` | `[Breakpoint, info registers]` | `[Validasi jalur trap dispatch]` |
| `[4]` | `[Dokumentasi Git]` | `[Branch, commit, push]` | `[Pengelolaan perubahan kode M4]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows 10 x64]` |
| Lingkungan build | `[WSL 2 Ubuntu]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-unknown-none-elf]` |
| Emulator | `[QEMU]` |
| Firmware emulator | `[Limine BIOS/UEFI]` |
| Debugger | `[GNU GDB]` |
| Build system | `[GNU Make]` |
| Bahasa utama | `[C17 Freestanding]` |
| Assembly | `[Digunakan pada M4 untuk ISR stub (isr.S)]` |

### 7.2 Versi Toolchain

Tempel output versi toolchain berikut. Jalankan dari clean shell WSL.

```bash
clang --version | head -n 1
ld.lld --version | head -n 1
readelf --version | head -n 1
objdump --version | head -n 1
nm --version | head -n 1
make --version | head -n 1
qemu-system-x86_64 --version | head -n 1
```

Output:

```text
Ubuntu clang version 21.1.8 (6ubuntu1)
Ubuntu LLD 21.1.8 (compatible with GNU linkers)
GNU readelf (GNU Binutils for Ubuntu) 2.46
GNU objdump (GNU Binutils for Ubuntu) 2.46
GNU nm (GNU Binutils for Ubuntu) 2.46
GNU Make 4.4.1
QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[~/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[m4-idt-exception-path]` |
| Commit hash awal | `` `[06de7f8]` `` |
| Commit hash akhir | `` `[82fccdf]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

Tampilkan hanya direktori dan file yang relevan dengan praktikum.

```text
mcsos/
├── kernel/
│   ├── core/
│   │   ├── kmain.c
│   │   └── trap.c
│   ├── include/
│   └── arch/
│       └── x86_64/
│           ├── idt.c
│           ├── isr.S
│           └── include/mcsos/arch/
│               ├── idt.h
│               └── isr.h
├── tools/
│   ├── scripts/
│   │   ├── m4_preflight.sh
│   │   ├── m4_audit_elf.sh
│   │   ├── m4_qemu_run.sh
│   │   ├── m4_collect_evidence.sh
│   │   └── grade_m4.sh
│   └── gdb_m4.gdb
├── evidence/
│   └── M4/
├── build/
├── Makefile
└── linker.ld
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[Makefile]` | `[ubah]` | `[Menambah target breakpoint, panic, inspect, dan audit untuk M4]` | `[sedang]` |
| `[kernel/core/kmain.c]` | `[ubah]` | `[Menambah inisialisasi IDT, selftest invariants, dan trigger exception untuk pengujian]` | `[sedang]` |
| `[kernel/core/trap.c]` | `[baru]` | `[Trap dispatcher untuk menangani exception]` | `[sedang]` |
| `[kernel/arch/x86_64/idt.c]` | `[baru]` | `[Inisialisasi dan pemuatan Interrupt Descriptor Table]` | `[sedang]` |
| `[kernel/arch/x86_64/isr.S]` | `[baru]` | `[Stub ISR assembly untuk setiap vector exception]` | `[sedang]` |
| `[kernel/arch/x86_64/include/mcsos/arch/idt.h]` | `[baru]` | `[Header struktur IDT entry]` | `[rendah]` |
| `[kernel/arch/x86_64/include/mcsos/arch/isr.h]` | `[baru]` | `[Header deklarasi ISR stub]` | `[rendah]` |
| `[kernel/include/mcsos/kernel/version.h]` | `[ubah]` | `[Memperbarui milestone menjadi M4]` | `[rendah]` |
| `[tools/gdb_m4.gdb]` | `[baru]` | `[Script debugging M4]` | `[rendah]` |
| `[tools/scripts/m4_preflight.sh]` | `[baru]` | `[Memeriksa kesiapan toolchain dan readiness M0-M3]` | `[rendah]` |
| `[tools/scripts/m4_audit_elf.sh]` | `[baru]` | `[Audit ELF, symbol IDT, LIDT, dan IRETQ]` | `[rendah]` |
| `[tools/scripts/m4_qemu_run.sh]` | `[baru]` | `[QEMU smoke test dan validasi log serial]` | `[rendah]` |
| `[tools/scripts/m4_collect_evidence.sh]` | `[baru]` | `[Mengumpulkan evidence M4]` | `[rendah]` |
| `[tools/scripts/grade_m4.sh]` | `[baru]` | `[Grading lokal M4]` | `[rendah]` |

### 8.3 Ringkasan Diff

```text
git status --short
git diff --stat
git log --oneline -n 5
```

```text
19 files changed, 1829 insertions(+), 19 deletions(-)
```

---

## 9. Desain Teknis

### 9.1 Masalah yang Diselesaikan

```text
Kernel pada milestone sebelumnya (M3) hanya mampu melakukan boot, logging, dan panic path sederhana tanpa mekanisme interrupt/exception yang nyata. Pada praktikum M4 ditambahkan Interrupt Descriptor Table (IDT), stub ISR assembly untuk setiap vector exception, serta trap dispatcher dalam C sehingga kernel mampu menangkap exception CPU (seperti breakpoint #BP) secara terstruktur dan mencetak informasi register sebelum kembali atau melakukan panic.
```

### 9.2 Keputusan Desain

| Keputusan | Alternatif yang dipertimbangkan | Alasan memilih | Konsekuensi |
|---|---|---|---|
| `[IDT statis 256 entry di kernel]` | `[IDT dinamis dialokasikan di heap]` | `[Belum ada kernel heap pada M4]` | `[Ukuran IDT tetap, limit tetap 4095]` |
| `[ISR stub dalam assembly terpisah (isr.S)]` | `[Stub ditulis langsung dalam C menggunakan inline asm]` | `[Kontrol penuh terhadap urutan push/pop register dan stack frame]` | `[Menambah berkas assembly baru yang harus dirawat terpisah]` |
| `[Trap dispatch tunggal (x86_64_trap_dispatch)]` | `[Handler terpisah per vector]` | `[Lebih sederhana untuk diaudit dan diuji]` | `[Logika percabangan vector dilakukan di satu fungsi]` |
| `[Pengujian breakpoint terkontrol via flag build]` | `[Memicu exception nyata tanpa flag]` | `[Memudahkan pemisahan build normal, breakpoint, dan panic]` | `[Perlu tiga varian build (normal/breakpoint/panic)]` |

### 9.3 Arsitektur Ringkas

Tambahkan diagram ASCII atau Mermaid. Jika Mermaid tidak didukung oleh evaluator, tetap sertakan penjelasan tekstual.

```mermaid
Bootloader
     │
     ▼
Kernel (kmain)
     │
     ├──► x86_64_idt_init (LIDT)
     │
     ├──► m4_selftest (invariant IDT)
     │
     ├──► [opsional] trigger breakpoint (#BP)
     │            │
     │            ▼
     │      ISR stub (isr.S)
     │            │
     │            ▼
     │    x86_64_trap_dispatch
     │            │
     │            ▼
     │       Serial log register
     │            │
     │            ▼
     │          IRETQ
     │
     └──► [opsional] kernel_panic_at (jalur panic)
```

Penjelasan diagram:

```text
Kernel dijalankan oleh bootloader melalui fungsi kmain(). Kernel kemudian memanggil x86_64_idt_init() untuk membangun dan memuat IDT menggunakan instruksi LIDT. Setelah itu dijalankan m4_selftest() untuk memverifikasi invariant ukuran entry IDT, base, dan limit. Jika dikompilasi dengan flag pengujian, kernel memicu exception breakpoint (#BP) secara sengaja sehingga CPU melompat ke ISR stub pada isr.S, yang kemudian memanggil x86_64_trap_dispatch() untuk mencetak informasi vector, error code, RIP, CS, RFLAGS, dan register umum ke serial log sebelum mengeksekusi IRETQ untuk kembali ke kmain(). Jika flag panic diaktifkan, kernel akan memanggil kernel_panic_at() sebagai jalur panic.
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[x86_64_idt_init()]` | `[kmain]` | `[IDT/CPU]` | `[Kernel sudah masuk long mode]` | `[IDT termuat dan aktif]` | `[Tidak ada fallback, kegagalan berarti bug build]` |
| `[x86_64_trap_dispatch()]` | `[ISR stub (isr.S)]` | `[Kernel]` | `[Exception terjadi, context tersimpan di stack]` | `[Informasi trap tercetak, kontrol kembali ke ISR stub]` | `[kernel_panic_at jika exception fatal]` |
| `[x86_64_trigger_breakpoint_for_test()]` | `[kmain]` | `[CPU]` | `[IDT sudah termuat]` | `[Exception #BP tertangkap dan ditangani]` | `[panic jika handler tidak terpasang]` |
| `[kernel_panic_at()]` | `[trap dispatch/kmain]` | `[panic handler]` | `[terjadi panic]` | `[informasi panic tercetak]` | `[halt]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `[x86_64_idt_entry_t]` `` | `[offset, selector, ist, type_attr]` | `[kernel]` | `[Selama kernel aktif]` | `[Ukuran tetap 16 byte]` |
| `` `[IDT table (256 entry)]` `` | `[Array of x86_64_idt_entry_t]` | `[kernel]` | `[Selama kernel aktif]` | `[Limit selalu 4095]` |
| `` `[trap frame]` `` | `[trap_vector, trap_error, trap_rip, trap_cs, trap_rflags, register umum]` | `[kernel]` | `[Selama exception ditangani]` | `[Selalu lengkap sebelum dispatch dipanggil]` |

### 9.6 Invariants

Tuliskan invariant yang harus benar sepanjang eksekusi.

1. `Ukuran x86_64_idt_entry_t harus selalu 16 byte.`
2. `idt_limit_for_test() harus selalu bernilai 4095 (256 entry x 16 byte - 1).`
3. `IDT harus termuat (LIDT) sebelum exception apapun dipicu.`
4. `Setiap ISR stub harus mengembalikan eksekusi melalui IRETQ.`
5. `Tidak terdapat undefined symbol pada kernel ELF normal, breakpoint, maupun panic.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[IDT]` | `[kernel]` | `[none]` | `[ya]` | `[Dimuat sekali saat init]` |
| `[serial port/log buffer]` | `[kernel]` | `[none]` | `[ya]` | `[Digunakan dari trap dispatch]` |

Lock order yang berlaku:

```text
Belum menggunakan mekanisme locking karena kernel masih berjalan pada konfigurasi single-core sehingga belum terdapat akses bersamaan terhadap IDT maupun resource trap dispatch.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Layout IDT entry salah]` | `[idt.c]` | `[Selftest ukuran entry dan limit]` | `[m4_selftest, KERNEL_ASSERT]` |
| `[Stack tidak konsisten saat masuk/keluar ISR]` | `[isr.S]` | `[Urutan push/pop register dijaga konsisten dengan trap dispatch]` | `[objdump disassembly]` |
| `[Undefined symbol]` | `[Linker]` | `[Audit symbol nm -u]` | `[nm, readelf, m4_audit_elf.sh]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[Bootloader → Kernel]` | `[ELF]` | `[Linker & readelf]` | `[panic]` |
| `[CPU exception → Trap dispatch]` | `[trap_vector, trap_error]` | `[Dicetak dan diverifikasi via log]` | `[kernel_panic_at untuk exception fatal]` |
| `[GDB Stub]` | `[Register]` | `[Breakpoint]` | `[Halt]` |

---

## 10. Langkah Kerja Implementasi

Gunakan tabel berikut untuk setiap langkah. Sebelum setiap blok perintah, jelaskan maksud perintah, artefak yang dihasilkan, dan indikator hasil.

### Langkah 1 — `Menjalankan preflight M4`

Maksud langkah:

```text
Memastikan toolchain (QEMU, clang, ld.lld, readelf) tersedia dan readiness M0-M3 terpenuhi sebelum implementasi M4 dimulai.
```

Perintah:

```bash
chmod +x tools/scripts/m4_preflight.sh
./tools/scripts/m4_preflight.sh
```

Output ringkas:

```text
[M4][PASS] QEMU tersedia: QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
[M4][PASS] clang: Ubuntu clang version 21.1.8 (6ubuntu1)
[M4][PASS] ld.lld: Ubuntu LLD 21.1.8 (compatible with GNU linkers)
[M4][PASS] readelf: GNU readelf (GNU Binutils for Ubuntu) 2.46
[M4][PASS] M0/M1/M2/M3 readiness minimum untuk M4 terpenuhi.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[tools/scripts/m4_preflight.sh]` | `[tools/scripts]` | `[Memverifikasi kesiapan toolchain dan readiness sebelumnya]` |

Indikator berhasil:

```text
Seluruh pemeriksaan preflight M4 berhasil tanpa error (PASS).
```

### Langkah 2 — `Membuat berkas IDT, ISR, dan trap dispatch`

Maksud langkah:

```text
Membuat header dan implementasi IDT (idt.h, idt.c), header dan stub ISR assembly (isr.h, isr.S), serta trap dispatcher (trap.c) sebagai dasar penanganan exception M4.
```

Perintah:

```bash
mkdir -p kernel/arch/x86_64/include/mcsos/arch
nano kernel/arch/x86_64/include/mcsos/arch/idt.h
nano kernel/arch/x86_64/include/mcsos/arch/isr.h
nano kernel/arch/x86_64/idt.c
nano kernel/arch/x86_64/isr.S
nano kernel/core/trap.c
nano kernel/core/kmain.c
nano kernel/include/mcsos/kernel/version.h
```

Output ringkas:

```text
File idt.h, isr.h, idt.c, isr.S, trap.c berhasil dibuat dan kmain.c serta version.h diperbarui untuk milestone M4.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[idt.h, idt.c]` | `[kernel/arch/x86_64]` | `[Struktur dan inisialisasi IDT]` |
| `[isr.h, isr.S]` | `[kernel/arch/x86_64]` | `[Stub ISR assembly]` |
| `[trap.c]` | `[kernel/core]` | `[Trap dispatcher]` |

Indikator berhasil:

```text
Seluruh berkas baru berhasil dibuat dan dapat dikompilasi pada langkah berikutnya.
```

### Langkah 3 — `Memperbarui Makefile untuk mendukung assembly dan target M4`

Maksud langkah:

```text
Menambahkan rule kompilasi berkas .S, target breakpoint, serta memperluas target inspect dan audit agar memeriksa symbol dan instruksi terkait IDT/exception.
```

Perintah:

```bash
nano Makefile
grep -n "SRC_S" Makefile
grep -n "%.S" Makefile
grep -n "breakpoint" Makefile
```

Output ringkas:

```text
SRC_S := $(shell find kernel -name '*.S' | LC_ALL=C sort)
OBJ := ... $(patsubst %.S,$(BUILD_DIR)/normal/%.o,$(SRC_S))
BP_OBJ := ... $(patsubst %.S,$(BUILD_DIR)/breakpoint/%.o,$(SRC_S))
PANIC_OBJ := ... $(patsubst %.S,$(BUILD_DIR)/panic/%.o,$(SRC_S))
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[Makefile]` | `[root repo]` | `[Mengatur build normal, breakpoint, panic, inspect, dan audit]` |

Indikator berhasil:

```text
Rule kompilasi assembly dan target breakpoint berhasil ditambahkan tanpa merusak target lama.
```

### Langkah 4 — `Build kernel normal`

Maksud langkah:

```text
Mengompilasi kernel M4 (idt.c, kmain.c, log.c, panic.c, serial.c, trap.c, memory.c, isr.S) menjadi file ELF utama.
```

Perintah:

```bash
make clean
make build
```

Output ringkas:

```text
ld.lld -nostdlib -static -z max-page-size=0x1000 -T linker.ld -Map=build/kernel.map -o build/kernel.elf \
build/normal/kernel/arch/x86_64/idt.o build/normal/kernel/core/kmain.o build/normal/kernel/core/log.o \
build/normal/kernel/core/panic.o build/normal/kernel/core/serial.o build/normal/kernel/core/trap.o \
build/normal/kernel/lib/memory.o build/normal/kernel/arch/x86_64/isr.o
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[kernel.elf]` | `[build]` | `[Kernel utama M4]` |
| `[kernel.map]` | `[build]` | `[informasi alamat simbol]` |

Indikator berhasil:

```text
File kernel.elf dan kernel.map berhasil dibuat tanpa error linker.
```

### Langkah 5 — `Inspect dan Audit ELF (target Makefile)`

Maksud langkah:

```text
Memeriksa header ELF, symbol IDT/trap dispatch, dan instruksi LIDT/IRETQ pada hasil disassembly, kemudian membangun varian breakpoint dan panic untuk diaudit bersama.
```

Perintah:

```bash
make inspect
make audit
```

Output ringkas:

```text
grep -q 'x86_64_idt_init' build/kernel.syms.txt
grep -q 'x86_64_trap_dispatch' build/kernel.syms.txt
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
! nm -u build/kernel.elf | grep .
! nm -u build/kernel.breakpoint.elf | grep .
! nm -u build/kernel.panic.elf | grep .
grep -q 'isr_stub_14' build/kernel.syms.txt
grep -q 'x86_64_exception_stubs' build/kernel.syms.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[kernel.readelf.header.txt]` | `[build]` | `[Header ELF]` |
| `[kernel.readelf.programs.txt]` | `[build]` | `[program header]` |
| `[kernel.syms.txt]` | `[build]` | `[daftar simbol, termasuk IDT/trap]` |
| `[kernel.disasm.txt]` | `[build]` | `[Disassembly kernel, termasuk LIDT/IRETQ]` |
| `[kernel.breakpoint.elf, kernel.panic.elf]` | `[build]` | `[varian kernel untuk pengujian exception]` |

Indikator berhasil:

```text
Seluruh proses inspect dan audit selesai tanpa ditemukan undefined symbol pada ketiga varian kernel.
```

### Langkah 6 — `Membuat dan melengkapi script tools M4`

Maksud langkah:

```text
Membuat script pendukung M4: audit ELF mandiri, QEMU smoke test, pengumpulan evidence, grading lokal, dan script GDB.
```

Perintah:

```bash
touch tools/scripts/m4_audit_elf.sh
touch tools/scripts/m4_qemu_run.sh
touch tools/scripts/m4_collect_evidence.sh
touch tools/scripts/grade_m4.sh
touch tools/gdb_m4.gdb
chmod +x tools/scripts/m4_audit_elf.sh tools/scripts/m4_qemu_run.sh \
  tools/scripts/m4_collect_evidence.sh tools/scripts/grade_m4.sh
nano tools/scripts/m4_audit_elf.sh
nano tools/scripts/m4_qemu_run.sh
nano tools/scripts/m4_collect_evidence.sh
nano tools/scripts/grade_m4.sh
nano tools/gdb_m4.gdb
```

Output ringkas:

```text
tools/scripts/m4_audit_elf.sh, m4_qemu_run.sh, m4_collect_evidence.sh,
grade_m4.sh, dan tools/gdb_m4.gdb berhasil dibuat dan diberi izin eksekusi.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m4_audit_elf.sh]` | `[tools/scripts]` | `[Audit ELF, symbol IDT, LIDT, IRETQ]` |
| `[m4_qemu_run.sh]` | `[tools/scripts]` | `[Menjalankan QEMU smoke test dan validasi log]` |
| `[m4_collect_evidence.sh]` | `[tools/scripts]` | `[Mengumpulkan evidence ke evidence/M4]` |
| `[grade_m4.sh]` | `[tools/scripts]` | `[Grading lokal M4]` |
| `[gdb_m4.gdb]` | `[tools]` | `[Script debugging GDB M4]` |

Indikator berhasil:

```text
Seluruh script dapat dieksekusi dan menjalankan pemeriksaan masing-masing tanpa error syntax.
```

### Langkah 7 — `Menjalankan audit ELF mandiri (m4_audit_elf.sh)`

Maksud langkah:

```text
Memvalidasi kernel.elf secara independen dari Makefile menggunakan script m4_audit_elf.sh.
```

Perintah:

```bash
./tools/scripts/m4_audit_elf.sh build/kernel.elf
```

Output ringkas:

```text
[M4][PASS] ELF, symbol, IDT, LIDT, dan IRETQ audit lulus untuk build/kernel.elf
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m4.readelf.header.txt, m4.readelf.programs.txt, m4.readelf.sections.txt]` | `[build]` | `[Hasil readelf untuk audit mandiri]` |
| `[m4.syms.txt]` | `[build]` | `[Daftar symbol audit mandiri]` |
| `[m4.disasm.txt]` | `[build]` | `[Disassembly audit mandiri]` |

Indikator berhasil:

```text
Audit ELF mandiri menyatakan PASS untuk seluruh pemeriksaan ELF, symbol, LIDT, dan IRETQ.
```

### Langkah 8 — `Membuat image ISO`

Maksud langkah:

```text
Membuat file ISO berisi kernel sehingga dapat dijalankan pada QEMU menggunakan bootloader Limine.
```

Perintah:

```bash
./tools/scripts/make_iso.sh
```

Output ringkas:

```text
ISO image produced: 2105 sectors
Limine BIOS stages installed successfully.
OK: ISO dibuat pada build/mcsos.iso
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[mcsos.iso]` | `[build]` | `[image boot kernel M4]` |

Indikator berhasil:

```text
File build/mcsos.iso berhasil dibuat dengan checksum SHA-256 yang tercetak pada terminal.
```

### Langkah 9 — `Menjalankan QEMU smoke test`

Maksud langkah:

```text
Menjalankan kernel pada QEMU untuk memastikan IDT termuat dan jalur dispatch exception berjalan sesuai harapan.
```

Perintah:

```bash
./tools/scripts/m4_qemu_run.sh
```

Output ringkas:

```text
[M4][FAIL] ISO tidak ditemukan: build/mcsos.iso
```

(Setelah ISO dibuat pada Langkah 8, smoke test dijalankan ulang dengan menambahkan nama log eksplisit untuk varian breakpoint.)

```bash
./tools/scripts/m4_qemu_run.sh build/mcsos.iso build/m4-qemu-breakpoint.log
```

Output ringkas:

```text
qemu-system-x86_64: terminating on signal 15 from pid 6833 (timeout)
[M4][PASS] QEMU smoke test lulus. Log: build/m4-qemu-breakpoint.log
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m4-qemu-breakpoint.log]` | `[build]` | `[Log serial kernel saat exception breakpoint dipicu]` |

Indikator berhasil:

```text
Kernel berhasil boot, memuat IDT, memicu dan menangani exception breakpoint, lalu kembali ke jalur eksekusi normal.
```

### Langkah 10 — `Debugging menggunakan GDB`

Maksud langkah:

```text
Menjalankan QEMU dengan gdbstub (-S -s) lalu menghubungkan GDB untuk memeriksa breakpoint pada kmain(), x86_64_idt_init(), dan x86_64_trap_dispatch() beserta register CPU.
```

Perintah:

```bash
qemu-system-x86_64 \
  -machine q35 \
  -cpu max \
  -m 256M \
  -cdrom build/mcsos.iso \
  -boot d \
  -serial stdio \
  -display none \
  -no-reboot \
  -no-shutdown \
  -S -s
gdb -q -x tools/gdb_m4.gdb
```

Output ringkas:

```text
Breakpoint 1, 0xffffffff80000210 in kmain ()
Breakpoint 2, 0xffffffff800000c0 in x86_64_idt_init ()
Breakpoint 3, 0xffffffff80000960 in x86_64_trap_dispatch ()
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[gdb_m4.gdb]` | `[tools]` | `[Script debugging M4]` |

Indikator berhasil:

```text
Ketiga breakpoint (kmain, x86_64_idt_init, x86_64_trap_dispatch) berhasil tercapai dan informasi register (info registers) dapat ditampilkan pada setiap breakpoint.
```

### Langkah 11 — `Mengumpulkan evidence dan grading lokal`

Maksud langkah:

```text
Mengumpulkan seluruh artefak hasil praktikum M4 sebagai bukti implementasi serta menjalankan grading lokal otomatis.
```

Perintah:

```bash
./tools/scripts/m4_collect_evidence.sh
./tools/scripts/grade_m4.sh
```

Output ringkas:

```text
[M4][PASS] Evidence dikumpulkan di evidence/M4
M4_LOCAL_SCORE=90/100
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[evidence/M4/]` | `[evidence/M4]` | `[Bukti hasil praktikum: kernel.elf, kernel.map, syms, disasm, readelf, manifest, log]` |
| `[manifest.txt]` | `[evidence/M4]` | `[Metadata commit dan versi toolchain]` |

Indikator berhasil:

```text
Evidence berhasil dikumpulkan dan grading lokal memperoleh SCORE=90/100, dengan 10 poin belum tercapai karena nama log serial pada tahap pengumpulan evidence (build/m4-qemu-serial.log) belum sesuai dengan nama log hasil smoke test (build/m4-qemu-breakpoint.log).
```

### Langkah 12 — `Commit dan push ke GitHub`

Maksud langkah:

```text
Menyimpan seluruh perubahan implementasi M4 ke branch baru pada repository GitHub sebagai dokumentasi hasil praktikum.
```

Perintah:

```bash
git add Makefile linker.ld kernel tools evidence/M4
git commit -m "M4 add x86_64 IDT and exception trap path"
git push -u origin m4-idt-exception-path
```

Output ringkas:

```text
[m4-idt-exception-path 82fccdf] M4 add x86_64 IDT and exception trap path
19 files changed, 1829 insertions(+), 19 deletions(-)
To https://github.com/syifanurzimah/MCSOS.git
 * [new branch]      m4-idt-exception-path -> m4-idt-exception-path
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[commit 82fccdf]` | `[GitHub]` | `[Menyimpan hasil praktikum M4]` |
| `[branch m4-idt-exception-path]` | `[GitHub]` | `[Branch kerja M4]` |

Indikator berhasil:

```text
Perubahan berhasil dikomit dan branch m4-idt-exception-path berhasil dipush ke repository GitHub.
```

---

## 11. Checkpoint Buildable

Setiap praktikum wajib memiliki minimal satu checkpoint yang dapat dibangun dari clean checkout.

| Checkpoint | Perintah | Expected result | Status |
|---|---|---|---|
| Clean build | `` `make clean && make build` `` | `[kernel berhasil dibangun]` | `[PASS]` |
| Inspect symbol IDT/trap | `` `make inspect` `` | `[symbol x86_64_idt_init, x86_64_trap_dispatch, instruksi lidt/iretq ditemukan]` | `[PASS]` |
| Audit lengkap (normal/breakpoint/panic) | `` `make audit` `` | `[Tidak ada undefined symbol pada ketiga varian]` | `[PASS]` |
| image generation | `` `./tools/scripts/make_iso.sh` `` | `[build/mcsos.iso tersedia]` | `[PASS]` |
| QEMU smoke test | `` `./tools/scripts/m4_qemu_run.sh build/mcsos.iso build/m4-qemu-breakpoint.log` `` | `[Serial log menunjukkan IDT loaded dan trap dispatch]` | `[PASS]` |
| test suite | `` `./tools/scripts/grade_m4.sh` `` | `[SCORE=90/100]` | `[PARTIAL]` |

Catatan checkpoint:

```text
Seluruh tahapan implementasi M4 berhasil dijalankan. Kernel berhasil dibangun pada tiga varian (normal, breakpoint, panic), image ISO berhasil dibuat, QEMU dapat melakukan booting dan menangani exception breakpoint, debugging menggunakan GDB berjalan dengan baik pada tiga breakpoint, dan grading lokal memperoleh skor 90/100. Sepuluh poin belum tercapai karena ketidaksesuaian nama berkas log serial antara hasil m4_qemu_run.sh dan yang diperiksa oleh grade_m4.sh.
```

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

Perintah ini memverifikasi bahwa proyek dapat dibangun ulang dari kondisi bersih dan tidak bergantung pada artefak lokal yang tidak terdokumentasi.

```bash
make clean
make build
```

Hasil:

```text
Build berhasil tanpa error.
Menghasilkan file:
- build/kernel.elf
- build/kernel.map
```

Status: `[PASS]`

### 12.2 Static Inspection

Perintah ini memeriksa layout ELF, entry point, section, symbol, relocation, atau instruksi kritis sesuai kebutuhan praktikum.

```bash
readelf -h build/kernel.elf
readelf -l build/kernel.elf
readelf -S build/kernel.elf
nm -n build/kernel.elf
objdump -d -Mintel build/kernel.elf | head -n 120
```

Hasil penting:

```text
Class: ELF64
- Kernel berhasil dikenali sebagai ELF64.
- Architecture: x86-64.
- Section .text dan .rodata tersedia.
- Symbol kmain, x86_64_idt_init, x86_64_trap_dispatch, isr_stub_14, dan
  x86_64_exception_stubs ditemukan.
- Instruksi lidt dan iretq ditemukan pada hasil disassembly.
```

Status: `[PASS]`

### 12.3 QEMU Smoke Test

Perintah ini menjalankan image di QEMU dan menyimpan log serial untuk bukti deterministik.

```bash
./tools/scripts/m4_qemu_run.sh build/mcsos.iso build/m4-qemu-breakpoint.log
```

Hasil:

```text
Kernel berhasil dijalankan pada QEMU.
Serial log berhasil dibuat pada:
build/m4-qemu-breakpoint.log

[M4] IDT loaded
[M4] selftest: IDT invariants passed
[M4] triggering intentional breakpoint exception
[M4] trap dispatch: #BP Breakpoint
[M4] breakpoint handled; returning with iretq
[M4] returned from breakpoint handler
[M4] IDT and exception dispatch path installed
[M4] ready for QEMU smoke test and GDB audit
```

Status: `[PASS]`

### 12.4 GDB Debug Evidence

Perintah ini membuktikan bahwa kernel dapat di-debug dengan simbol yang cocok pada jalur IDT dan trap dispatch.

```bash
qemu-system-x86_64 -machine q35 -cpu max -m 256M -cdrom build/mcsos.iso \
  -boot d -serial stdio -display none -no-reboot -no-shutdown -S -s
gdb -q -x tools/gdb_m4.gdb
```

Hasil:

```text
Breakpoint 1, 0xffffffff80000210 in kmain ()
Breakpoint 2, 0xffffffff800000c0 in x86_64_idt_init ()
Breakpoint 3, 0xffffffff80000960 in x86_64_trap_dispatch ()

info registers menampilkan rip, rsp, rflags, cr0, cr3, cr4, efer
secara lengkap pada setiap breakpoint yang tercapai.
```

Status: `[PASS]`

### 12.5 Unit Test

```bash
./tools/scripts/grade_m4.sh
```

Hasil:

```text
M4_LOCAL_SCORE=90/100
```

Rincian skor sesuai grade_m4.sh:

```text
PASS[60]: make clean && make audit (build normal/breakpoint/panic + inspect + audit symbol)
PASS[20]: m4_audit_elf.sh build/kernel.elf
FAIL[10]: cek build/m4-qemu-serial.log (nama log tidak ditemukan)
PASS[10]: evidence/M4/manifest.txt tersedia
```

Status: `[PARTIAL]`

### 12.6 Stress/Fuzz/Fault Injection Test

Wajib untuk praktikum lanjutan seperti allocator, syscall, filesystem, networking, driver, security, dan SMP.

```bash
BELUM DILAKUKAN
```

Hasil:

```text
Tidak diterapkan pada milestone M4. Pengujian dilakukan secara terkontrol
hanya melalui flag build MCSOS_M4_TRIGGER_BREAKPOINT dan
MCSOS_M4_TRIGGER_PANIC, bukan fault injection acak.
```

Status: `[NA]`

### 12.7 Visual Evidence

Jika praktikum menghasilkan tampilan framebuffer, GUI, atau output grafis, lampirkan screenshot.

| Screenshot | Lokasi file | Keterangan |
|---|---|---|
| `[Screenshot build dan audit berhasil]` | `[lampiran]` | `[Build normal/breakpoint/panic berhasil]` |
| `[Screenshot grade_m4.sh]` | `[lampiran]` | `[Nilai M4_LOCAL_SCORE=90/100]` |
| `[Screenshot sesi GDB]` | `[lampiran]` | `[Breakpoint kmain, idt_init, trap_dispatch]` |
| `[Screenshot GitHub]` | `[lampiran]` | `[Branch m4-idt-exception-path setelah push]` |

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| No. | Uji | Expected result | Actual result | Status | Evidence |
|---|---|---|---|---|---|
| 1 | `[Build kernel normal]` | `[kernel berhasil dibangun]` | `[kernel.elf dan kernel.map berhasil dibuat]` | `[PASS]` | `[build/kernel.elf]` |
| 2 | `[Build kernel breakpoint dan panic]` | `[kedua varian berhasil dibangun]` | `[kernel.breakpoint.elf dan kernel.panic.elf berhasil dibuat]` | `[PASS]` | `[build/kernel.breakpoint.elf, build/kernel.panic.elf]` |
| 3 | `[Audit ELF, symbol IDT/trap, LIDT, IRETQ]` | `[Header, symbol, dan disassembly valid]` | `[Semua pemeriksaan berhasil]` | `[PASS]` | `[build/kernel.readelf.*, kernel.syms.txt, kernel.disasm.txt]` |
| 4 | `[QEMU smoke test exception breakpoint]` | `[IDT loaded dan trap dispatch tercatat]` | `[Log menunjukkan #BP Breakpoint tertangani dan IRETQ]` | `[PASS]` | `[build/m4-qemu-breakpoint.log]` |
| 5 | `[Debugging GDB pada kmain, idt_init, trap_dispatch]` | `[Ketiga breakpoint tercapai]` | `[Ketiga breakpoint berhasil dicapai dan register terbaca]` | `[PASS]` | `[sesi GDB]` |
| 6 | `[Evidence Collection]` | `[Evidence tersimpan]` | `[Semua file berhasil disalin ke evidence/M4]` | `[PASS]` | `[evidence/M4]` |
| 7 | `[Grading lokal]` | `[skor penuh]` | `[SCORE 90/100]` | `[PARTIAL]` | `[grade_m4.sh]` |

### 13.2 Log Penting

```text
[M4] IDT loaded
[M4] selftest: IDT invariants passed
[M4] triggering intentional breakpoint exception
[M4] trap dispatch: #BP Breakpoint
trap_vector=0x0000000000000003
trap_error=0x0000000000000000
trap_rip=0xffffffff80000205
trap_cs=0x0000000000000028
trap_rflags=0x0000000000000082
[M4] breakpoint handled; returning with iretq
[M4] returned from breakpoint handler
[M4] IDT and exception dispatch path installed
[M4] ready for QEMU smoke test and GDB audit
M4_LOCAL_SCORE=90/100
```

### 13.3 Artefak Bukti

| Artefak | Path | SHA-256 / hash | Fungsi |
|---|---|---|---|
| `kernel.elf` | `[build/kernel.elf]` | `[-]` | `[kernel utama M4]` |
| `kernel.map` | `[build/kernel.map]` | `[-]` | `[linker map]` |
| `kernel.breakpoint.elf` | `[build/kernel.breakpoint.elf]` | `[-]` | `[kernel pengujian exception breakpoint]` |
| `kernel.panic.elf` | `[build/kernel.panic.elf]` | `[-]` | `[kernel pengujian jalur panic M4]` |
| `kernel.readelf.header.txt` | `[build/kernel.readelf.header.txt]` | `[-]` | `[Header ELF]` |
| `kernel.readelf.programs.txt` | `[build/kernel.readelf.programs.txt]` | `[-]` | `[program header]` |
| `kernel.disasm.txt` | `[build/kernel.disasm.txt]` | `[-]` | `[disassembly, termasuk lidt/iretq]` |
| `kernel.syms.txt` | `[build/kernel.syms.txt]` | `[-]` | `[daftar symbol, termasuk IDT/trap]` |
| `mcsos.iso` | `[build/mcsos.iso]` | `[20af4449b56ebef05f14ade34c3da9b6b7b59f9e4954e32e662a24a56711d5a8]` | `[image boot kernel]` |
| `manifest.txt` | `[evidence/M4/manifest.txt]` | `[-]` | `[metadata commit dan toolchain]` |

Perintah hash:

```bash
sha256sum build/mcsos.iso
```

---

## 14. Analisis Teknis

### 14.1 Analisis Keberhasilan

```text
Praktikum M4 berhasil dilaksanakan dengan baik. Kernel berhasil dibangun pada tiga varian (normal, breakpoint, panic), symbol IDT (x86_64_idt_init), trap dispatch (x86_64_trap_dispatch), serta stub ISR (isr_stub_14, x86_64_exception_stubs) berhasil ditemukan pada audit ELF. Instruksi LIDT dan IRETQ ditemukan pada hasil disassembly, menunjukkan bahwa kernel benar-benar memuat IDT dan mengembalikan eksekusi exception dengan benar. QEMU smoke test menunjukkan jalur exception breakpoint berjalan sesuai rancangan, dan sesi GDB berhasil menjangkau breakpoint pada kmain(), x86_64_idt_init(), serta x86_64_trap_dispatch() dengan informasi register yang lengkap.
```

### 14.2 Analisis Kegagalan atau Perbedaan Hasil

```text
Selama proses pengerjaan ditemukan beberapa kendala, antara lain script tools/scripts/m4_audit_elf.sh, m4_qemu_run.sh, m4_collect_evidence.sh, dan grade_m4.sh yang awalnya belum dibuat (hanya disinggung pada preflight), serta ketidaksesuaian nama berkas log serial: m4_qemu_run.sh dijalankan dengan nama log build/m4-qemu-breakpoint.log, sedangkan grade_m4.sh dan m4_collect_evidence.sh memeriksa nama build/m4-qemu-serial.log. Akibatnya, hasil grading lokal hanya mencapai SCORE=90/100, dengan 10 poin terkait pengecekan log serial yang belum terpenuhi. Permasalahan ini telah teridentifikasi dan dapat diperbaiki dengan menyamakan nama log antar script pada perbaikan berikutnya.
```

### 14.3 Perbandingan dengan Teori

| Konsep teori | Implementasi praktikum | Sesuai/tidak sesuai | Penjelasan |
|---|---|---|---|
| `[Interrupt Descriptor Table]` | `[x86_64_idt_init membangun dan memuat IDT 256 entry]` | `[sesuai]` | `[idt_limit_for_test() bernilai 4095 sesuai 256 x 16 byte - 1]` |
| `[Instruksi LIDT]` | `[Ditemukan pada hasil objdump]` | `[sesuai]` | `[Menunjukkan IDT benar-benar dimuat ke CPU]` |
| `[Exception #BP (breakpoint)]` | `[x86_64_trigger_breakpoint_for_test memicu exception, ditangkap isr_stub]` | `[sesuai]` | `[Log serial menunjukkan trap_vector=0x3 yaitu vector #BP]` |
| `[Instruksi IRETQ]` | `[Ditemukan pada hasil objdump dan log "returning with iretq"]` | `[sesuai]` | `[Menunjukkan handler exception mengembalikan eksekusi dengan benar]` |

### 14.4 Kompleksitas dan Kinerja

| Aspek | Estimasi/hasil | Bukti | Catatan |
|---|---|---|---|
| Kompleksitas algoritma | `[O(1)]` | `[Analisis kode idt.c dan trap.c]` | `[Inisialisasi IDT bersifat tabel statis, tidak ada algoritma kompleks]` |
| Waktu build | `[kurang dari 1 menit]` | `[output make]` | `[Build normal, breakpoint, dan panic berhasil cepat]` |
| Waktu boot QEMU | `[beberapa detik hingga timeout 20 detik]` | `[serial log]` | `[Kernel halt setelah selesai sehingga QEMU dihentikan via timeout]` |
| Penggunaan memori | `[256 MB]` | `[konfigurasi QEMU -m 256M]` | `[sesuai konfigurasi script m4_qemu_run.sh]` |
| Latensi/throughput | `[TIDAK DIUKUR]` | `[-]` | `[belum menjadi fokus pada M4]` |

---

## 15. Debugging dan Failure Modes

### 15.1 Failure Modes yang Ditemukan

| Failure mode | Gejala | Penyebab sementara | Bukti | Perbaikan |
|---|---|---|---|---|
| `[Script tools/scripts/m4_audit_elf.sh kosong]` | `[Perintah gagal dijalankan, "No such file or directory"]` | `[Script belum dibuat]` | `[terminal]` | `[Membuat dan mengisi script menggunakan touch dan nano]` |
| `[ISO tidak ditemukan saat QEMU smoke test]` | `[M4][FAIL] ISO tidak ditemukan: build/mcsos.iso]` | `[Belum menjalankan make_iso.sh]` | `[m4_qemu_run.sh]` | `[Menjalankan tools/scripts/make_iso.sh terlebih dahulu]` |
| `[Grading lokal tidak mencapai 100/100]` | `[M4_LOCAL_SCORE=90/100]` | `[Nama log serial tidak konsisten antar script]` | `[grade_m4.sh, m4_collect_evidence.sh]` | `[Menyamakan nama log build/m4-qemu-serial.log pada seluruh script terkait]` |
| `[Permission denied saat mengedit kmain.c langsung]` | `[bash: kernel/core/kmain.c: Permission denied]` | `[Mengetik nama file tanpa perintah editor]` | `[terminal]` | `[Menggunakan nano kernel/core/kmain.c]` |

### 15.2 Failure Modes yang Diantisipasi

| Failure mode | Deteksi | Dampak | Mitigasi |
|---|---|---|---|
| `[Undefined symbol pada IDT/ISR]` | `[nm -u]` | `[kernel gagal link]` | `[Memastikan semua symbol IDT, ISR, dan trap tersedia]` |
| `[IDT tidak termuat sebelum exception]` | `[selftest x86_64_idt_base_for_test/limit]` | `[exception tidak tertangani, triple fault]` | `[Memanggil x86_64_idt_init sebelum trigger exception apapun]` |
| `[Kernel panic pada exception fatal]` | `[serial log]` | `[kernel berhenti]` | `[Menyediakan jalur kernel_panic_at sebagai fallback]` |

### 15.3 Triage yang Dilakukan

```text
1. Membuat dan mengisi seluruh script M4 yang sebelumnya kosong (touch + nano).
2. Memberikan izin eksekusi (chmod +x) pada seluruh script.
3. Menjalankan make clean, make build, make inspect, dan make audit secara berurutan.
4. Membangun image ISO menggunakan make_iso.sh sebelum menjalankan QEMU smoke test.
5. Menjalankan QEMU smoke test dengan nama log eksplisit (m4-qemu-breakpoint.log).
6. Menjalankan sesi debugging GDB untuk memverifikasi breakpoint kmain, x86_64_idt_init, dan x86_64_trap_dispatch.
7. Mengumpulkan evidence dan menjalankan grade_m4.sh hingga memperoleh SCORE=90/100.
8. Mengidentifikasi penyebab 10 poin yang hilang sebagai catatan perbaikan untuk iterasi berikutnya.
```

### 15.4 Panic Path

Jika terjadi panic, tempel output panic.

```text
Jalur panic M4 diuji melalui build panic (make panic / target panic pada audit), dengan flag MCSOS_M4_TRIGGER_PANIC=1 pada kmain.c yang memanggil KERNEL_PANIC("intentional M4 panic test", ...). Symbol kernel_panic_at berhasil ditemukan pada hasil audit ELF kernel.panic.elf, sehingga jalur panic M4 telah tersedia sebagai fallback apabila terjadi exception fatal di luar breakpoint terkontrol.
```

---

## 16. Prosedur Rollback

Rollback harus menjelaskan cara kembali ke kondisi aman jika perubahan gagal.

| Skenario rollback | Perintah | Data yang harus diselamatkan | Status |
|---|---|---|---|
| Kembali ke commit awal M3 | `` `git checkout 06de7f8` `` | `[Log build dan evidence M3]` | `[belum diuji]` |
| Revert implementasi M4 | `` `git revert 82fccdf` `` | `[Evidence M4 dan log pengujian]` | `[belum diuji]` |
| Bersihkan artefak build | `` `make clean` `` | `[Tidak ada, karena hanya menghapus hasil build]` | `[teruji]` |
| Regenerasi image | `` `./tools/scripts/make_iso.sh` `` | `[Image lama jika masih diperlukan]` | `[teruji]` |
| Berpindah branch ke main | `` `git switch main` `` | `[Branch m4-idt-exception-path tetap tersimpan di remote]` | `[teruji]` |

Catatan rollback:

```text
Pada praktikum ini rollback penuh menggunakan Git tidak dilakukan karena implementasi M4 berjalan dengan baik hingga memperoleh nilai SCORE=90/100 pada branch terpisah (m4-idt-exception-path), sehingga branch main tetap berada pada kondisi stabil M3. Mekanisme rollback telah disiapkan menggunakan git checkout/git revert apabila di kemudian hari diperlukan untuk kembali ke kondisi sebelumnya. Proses make clean telah diuji dan berhasil menghapus artefak hasil kompilasi tanpa memengaruhi source code.
```

---

## 17. Keamanan dan Reliability

### 17.1 Risiko Keamanan

| Risiko | Boundary | Dampak | Mitigasi | Evidence |
|---|---|---|---|---|
| `[Undefined symbol pada IDT/ISR/trap]` | `[linker]` | `[Kernel gagal dibangun]` | `[Pemeriksaan menggunakan nm -u dan audit ELF pada ketiga varian build]` | `[grade_m4.sh, m4_audit_elf.sh]` |
| `[IDT/Entry tidak valid sehingga exception tidak tertangkap]` | `[CPU saat menerima interrupt]` | `[Triple fault/reset CPU]` | `[Selftest invariant ukuran entry dan limit IDT sebelum exception dipicu]` | `[m4_selftest, KERNEL_ASSERT]` |
| `[Kesalahan layout ELF]` | `[Linker Script]` | `[Kernel gagal dijalankan]` | `[Verifikasi menggunakan readelf dan objdump]` | `[build/kernel.readelf.*, kernel.disasm.txt]` |

### 17.2 Reliability dan Data Integrity

| Risiko reliability | Dampak | Deteksi | Mitigasi |
|---|---|---|---|
| `[Build gagal pada salah satu varian (normal/breakpoint/panic)]` | `[Kernel tidak dapat diuji secara lengkap]` | `[Output make audit]` | `[Memperbaiki source dan menjalankan ulang make audit]` |
| `[Nama berkas log serial tidak konsisten antar script]` | `[Grading gagal mencapai skor penuh]` | `[grade_m4.sh, m4_collect_evidence.sh]` | `[Menyamakan nama log pada seluruh script terkait di iterasi berikutnya]` |
| `[ISO belum dibuat sebelum smoke test]` | `[QEMU smoke test gagal dijalankan]` | `[m4_qemu_run.sh]` | `[Menjalankan make_iso.sh sebelum smoke test]` |

### 17.3 Negative Test

| Negative test | Input buruk | Expected result | Actual result | Status |
|---|---|---|---|---|
| `[Build panic]` | `[make panic / flag MCSOS_M4_TRIGGER_PANIC=1]` | `[Panic path berhasil dikompilasi]` | `[kernel.panic.elf berhasil dibangun]` | `[PASS]` |
| `[Undefined symbol check]` | `[nm -u build/kernel.elf, kernel.breakpoint.elf, kernel.panic.elf]` | `[Tidak ada undefined symbol]` | `[Tidak ditemukan undefined symbol pada ketiga varian]` | `[PASS]` |
| `[QEMU smoke test sebelum ISO dibuat]` | `[./tools/scripts/m4_qemu_run.sh tanpa ISO]` | `[Gagal dengan pesan jelas]` | `[M4][FAIL] ISO tidak ditemukan: build/mcsos.iso]` | `[PASS]` |
| `[Audit ELF]` | `[File ELF diperiksa]` | `[semua pemeriksaan lolos]` | `[Audit berhasil]` | `[PASS]` |

---

## 18. Pembagian Kerja Kelompok

Isi bagian ini hanya jika praktikum dikerjakan berkelompok. Untuk pengerjaan individu, tulis "Tidak berlaku".

```text
Tidak berlaku, praktikum M4 dikerjakan secara individu.
```

| Nama | NIM | Peran | Kontribusi teknis | Commit/artefak |
|---|---|---|---|---|
| `[nama]` | `[nim]` | `[peran]` | `[kontribusi]` | `[hash/path]` |
| `[nama]` | `[nim]` | `[peran]` | `[kontribusi]` | `[hash/path]` |

### 18.1 Mekanisme Koordinasi

```text
Tidak berlaku karena pengerjaan dilakukan secara individu.
```

### 18.2 Evaluasi Kontribusi

| Anggota | Persentase kontribusi yang disepakati | Bukti | Catatan |
|---|---:|---|---|
| `[Syifa Nurzimah]` | `[100%]` | `[Repository Git, commit history, artefak build]` | `[praktikum individu]` |

---

## 19. Kriteria Lulus Praktikum

Bagian ini wajib diisi. Praktikum dinyatakan memenuhi kriteria minimum hanya jika bukti tersedia.

| Kriteria minimum | Status | Evidence |
|---|---|---|
| Proyek dapat dibangun dari clean checkout | `[PASS]` | `[make build]` |
| Perintah build terdokumentasi | `[PASS]` | `[bagian 10]` |
| QEMU boot atau test target berjalan deterministik | `[PASS]` | `[build/m4-qemu-breakpoint.log]` |
| Semua unit test/praktikum test relevan lulus | `[PARTIAL]` | `[M4_LOCAL_SCORE=90/100]` |
| Log serial disimpan | `[PASS]` | `[build/m4-qemu-breakpoint.log]` |
| Panic path terbaca atau dijelaskan jika belum relevan | `[PASS]` | `[kernel_panic_at, kernel.panic.elf]` |
| Tidak ada warning kritis pada build | `[PASS]` | `[make build, make audit]` |
| Perubahan Git terkomit | `[PASS]` | `[Commit 82fccdf]` |
| Desain dan failure mode dijelaskan | `[PASS]` | `[bagian 14 dan 15]` |
| Laporan berisi screenshot/log yang cukup | `[PASS]` | `[lampiran]` |

Kriteria tambahan untuk praktikum lanjutan:

| Kriteria lanjutan | Status | Evidence |
|---|---|---|
| Static analysis dijalankan | `[PASS]` | `[readelf, nm, objdump]` |
| Stress test dijalankan | `[NA]` | `[Tidak menjadi fokus M4]` |
| Fuzzing atau malformed-input test dijalankan | `[NA]` | `[Tidak menjadi fokus M4]` |
| Fault injection dijalankan | `[PASS]` | `[make panic, breakpoint test terkontrol]` |
| Disassembly/readelf evidence tersedia | `[PASS]` | `[evidence/M4]` |
| Review keamanan dilakukan | `[PASS]` | `[bagian 17]` |
| Rollback diuji | `[NA]` | `[Belum dilakukan karena implementasi sudah berhasil pada branch terpisah]` |

---

## 20. Readiness Review

Pilih satu status dengan alasan berbasis bukti.

| Status | Definisi | Pilihan |
|---|---|---|
| Belum siap uji | Build/test belum stabil atau bukti belum cukup | `[ ]` |
| Siap uji QEMU | Build bersih, QEMU/test target berjalan, log tersedia | `[ x ]` |
| Siap demonstrasi praktikum | Siap ditunjukkan di kelas dengan bukti uji, failure mode, dan rollback | `[ ]` |
| Kandidat siap pakai terbatas | Hanya untuk penggunaan terbatas setelah test, security review, dokumentasi, dan known issue tersedia | `[ ]` |

Alasan readiness:

```text
Berdasarkan hasil build normal, breakpoint, dan panic, audit ELF, pengumpulan evidence, sesi debugging GDB, serta grading lokal dengan hasil SCORE=90/100, implementasi M4 telah memenuhi sebagian besar pemeriksaan mekanis dan berhasil dijalankan pada QEMU dengan log deterministik. Namun masih terdapat satu known issue terkait konsistensi nama berkas log serial antar script sehingga praktikum dinyatakan siap uji QEMU, belum dinyatakan siap demonstrasi penuh.
```

Known issues:

| No. | Issue | Dampak | Workaround | Target perbaikan |
|---|---|---|---|---|
| 1 | `[Nama log serial build/m4-qemu-serial.log pada grade_m4.sh/m4_collect_evidence.sh tidak sama dengan nama log build/m4-qemu-breakpoint.log hasil m4_qemu_run.sh]` | `[Grading lokal hanya mencapai 90/100]` | `[Menjalankan m4_qemu_run.sh dengan nama log build/m4-qemu-serial.log secara eksplisit]` | `[M4 iterasi perbaikan]` |
| 2 | `[Belum terdapat pengujian stress/fuzz pada jalur exception]` | `[Tidak memengaruhi target minimum M4]` | `[Dilakukan pada milestone berikutnya]` | `[M5]` |

Keputusan akhir:

```text
Berdasarkan hasil pengujian dan evidence yang diperoleh, praktikum M4 dinyatakan siap uji QEMU. Seluruh proses build, audit ELF, exception breakpoint, debugging GDB, evidence collection, dan grading lokal berhasil dijalankan dengan hasil SCORE=90/100, dengan satu known issue minor terkait penamaan log serial yang telah didokumentasikan untuk perbaikan berikutnya.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Indikator nilai penuh | Nilai |
|---|---:|---|---:|
| Kebenaran fungsional | 30 | Implementasi memenuhi target praktikum, build/test lulus, output sesuai expected result | `[27]` |
| Kualitas desain dan invariants | 20 | Desain jelas, kontrak antarmuka eksplisit, invariants/ownership/locking terdokumentasi | `[20]` |
| Pengujian dan bukti | 20 | Unit/integration/QEMU/static/fuzz/stress evidence memadai sesuai tingkat praktikum | `[18]` |
| Debugging dan failure analysis | 10 | Failure mode, triage, panic/log, dan rollback dianalisis | `[10]` |
| Keamanan dan robustness | 10 | Boundary, input validation, privilege, memory safety, dan negative tests dibahas | `[10]` |
| Dokumentasi dan laporan | 10 | Laporan rapi, lengkap, dapat direproduksi, memakai referensi yang layak | `[10]` |
| **Total** | **100** |  | `[95]` |

Catatan penilai:

```text
[Diisi dosen/asisten.]
```

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Praktikum M4 berhasil menambahkan Interrupt Descriptor Table (IDT), stub ISR assembly, trap dispatcher, pengujian exception breakpoint terkontrol, audit ELF, debugging GDB, dan pengumpulan evidence. Kernel berhasil dibangun pada tiga varian (normal, breakpoint, panic), dan seluruh pemeriksaan utama (build, inspect, audit, QEMU smoke test, GDB) berhasil dijalankan dengan hasil grading lokal SCORE=90/100.
```

### 22.2 Yang Belum Berhasil

```text
Grading lokal belum mencapai skor penuh (100/100) karena ketidaksesuaian nama berkas log serial antara script m4_qemu_run.sh dan grade_m4.sh/m4_collect_evidence.sh. Selain itu, pengujian stress test, fuzzing, dan fault injection lanjutan pada jalur exception belum menjadi cakupan pada milestone M4 sehingga belum dilakukan.
```

### 22.3 Rencana Perbaikan

```text
Tahap selanjutnya adalah menyamakan nama berkas log serial pada seluruh script M4, melengkapi pengujian QEMU dengan nama log yang konsisten agar skor mencapai 100/100, serta melanjutkan implementasi M5 dengan menambahkan timer interrupt (PIT/APIC), manajemen memori awal, dan peningkatan kemampuan debugging kernel.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
git log --oneline -3
82fccdf (HEAD -> m4-idt-exception-path) M4 add x86_64 IDT and exception trap path
06de7f8 (main) Complete M3 panic logging baseline
9f8fbed (origin/main) M3 panic path logging gdb and disassembly audit
```

### Lampiran B — Diff Ringkas

```diff
git diff --stat
19 files changed, 1829 insertions(+), 19 deletions(-)
```

### Lampiran C — Log Build Lengkap

```text
Build berhasil tanpa error pada varian normal, breakpoint, dan panic.
```

### Lampiran D — Log QEMU Lengkap

```text
build/m4-qemu-breakpoint.log
```

### Lampiran E — Output Readelf/Objdump

```text
build/kernel.readelf.header.txt build/kernel.readelf.programs.txt
build/kernel.syms.txt build/kernel.disasm.txt
evidence/M4/kernel.readelf.header.txt evidence/M4/kernel.readelf.programs.txt
evidence/M4/kernel.syms.txt evidence/M4/kernel.disasm.txt
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[Screenshot Terminal]` | `[Build, audit, dan grading SCORE=90/100]` |
| 2 | `[Screenshot Sesi GDB]` | `[Breakpoint kmain, x86_64_idt_init, x86_64_trap_dispatch]` |
| 3 | `[Screenshot GitHub]` | `[Branch m4-idt-exception-path setelah push]` |

### Lampiran G — Bukti Tambahan

```text
evidence/M4/
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
[1] Intel Corporation, Intel® 64 and IA-32 Architectures Software Developer's Manual, "Interrupt and Exception Handling" chapter.
[2] QEMU Project Documentation, "GDB usage" (-s -S gdbstub).
[3] GNU Binutils Documentation, readelf, objdump, dan nm.
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
| Log QEMU/test dilampirkan | `[Ya]` |
| Artefak penting diberi hash | `[Ya]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya/kami mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
Saya mengumpulkan laporan ini bersama artefak pendukung pada commit:

82fccdf

Status akhir yang diklaim:

Siap uji QEMU
```

Status akhir yang diklaim:

```text
[Siap uji QEMU]
```

Ringkasan satu paragraf:

```text
[Praktikum M4 berhasil diimplementasikan dengan menambahkan Interrupt Descriptor Table (IDT), stub ISR assembly, trap dispatcher, pengujian exception breakpoint terkontrol, audit ELF, debugging GDB, dan pengumpulan evidence. Seluruh proses build (normal/breakpoint/panic), audit, QEMU smoke test, dan grading lokal berhasil dijalankan dengan hasil SCORE=90/100. Repository telah diperbarui pada branch m4-idt-exception-path di GitHub beserta seluruh perubahan yang diperlukan sebagai bukti penyelesaian praktikum.]
```
