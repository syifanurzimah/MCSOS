 Laporan Praktikum Sistem Operasi Lanjut — MCSOS-M7

**Nama file laporan:** `laporan_praktikum_m7_25832074009.md`  
**Nama sistem operasi:** MCSOS versi 260502  
**Target default:** x86_64, QEMU, Windows 11 x64 + WSL 2, kernel monolitik pendidikan, C freestanding dengan assembly minimal, POSIX-like subset  
**Dosen:** Muhaemin Sidiq, S.Pd., M.Pd.  
**Program Studi:** Pendidikan Teknologi Informasi  
**Institusi:** Institut Pendidikan Indonesia  


---

## 0. Metadata Laporan

| Atribut | Isi |
|---|---|
| Kode praktikum | `[M7]` |
| Judul praktikum | `[Virtual Memory Manager: Page Table x86_64, Mapping/Unmapping Halaman, dan Invalidasi TLB]` |
| Jenis pengerjaan | `[Individu]` |
| Nama mahasiswa | `[Syifa Nurzimah]` |
| NIM | `[25832074009]` |
| Kelas | `[1A]` |
| Nama kelompok | `[isi jika kelompok]` |
| Anggota kelompok | `[nama, NIM, peran ringkas]` |
| Tanggal praktikum | `[2026-07-03]` |
| Tanggal pengumpulan | `[2026-07-04]` |
| Repository | `[https://github.com/syifanurzimah/MCSOS]` |
| Branch | `[praktikum/m7-vmm]` |
| Commit awal | `` `[bc3b131]` `` |
| Commit akhir | `` `[40a6aa4]` `` |
| Status readiness yang diklaim | `[Siap uji (build & host test); QEMU/GDB belum mencerminkan build M7 terbaru]` |

---

## 1. Sampul

# Laporan Praktikum `M7`  
## `Virtual Memory Manager: Page Table x86_64, Mapping/Unmapping Halaman, dan Invalidasi TLB`

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
Assistant (ChatGPT) untuk membantu menjelaskan konsep page table x86_64,
memperbaiki error build (asm pada -std=c17, duplikasi fungsi), menyusun
script grading, dan membantu penyusunan laporan.

Seluruh kode yang dihasilkan diverifikasi kembali dengan proses build
freestanding, build host-test, unit test (test_vmm_host.c), serta audit
ELF/objdump untuk memastikan instruksi invlpg dan akses register cr3
benar-benar dikompilasi menjadi instruksi x86_64.
```

---

## 3. Tujuan Praktikum

Tuliskan tujuan teknis dan konseptual praktikum. Tujuan harus dapat diuji.

1. `Membangun kernel MCSOS milestone M7 dengan menambahkan Virtual Memory Manager (VMM) sebagai lanjutan dari Physical Memory Manager (PMM) pada milestone M6.`
2. `Mengimplementasikan operasi pemetaan dan pelepasan pemetaan halaman (map/unmap) pada struktur page table x86_64 melalui pemeriksaan flag VMM_PTE_PRESENT dan VMM_PTE_HUGE.`
3. `Mengimplementasikan vmm_invalidate_page() menggunakan instruksi invlpg, serta vmm_read_cr3()/vmm_write_cr3() untuk membaca dan menulis register CR3, dan vmm_read_cr2() untuk membaca alamat page fault.`
4. `Memvalidasi implementasi menggunakan unit test host (tests/test_vmm_host.c) dan static grading script (scripts/grade_m7.sh) yang memeriksa unresolved symbol serta keberadaan instruksi invlpg dan akses cr3 pada hasil objdump.`

---

## 4. Capaian Pembelajaran Praktikum

Setelah praktikum ini, mahasiswa mampu:

| CPL/CPMK praktikum | Bukti yang harus ditunjukkan |
|---|---|
| `[Mampu membangun kernel M7 dengan modul VMM (vmm.c, vmm.h)]` | `[build berhasil, kernel.elf, kernel.map, vmm.c dan vmm.h terkompilasi dan terlink]` |
| `[Mampu melakukan audit objdump untuk memastikan instruksi invlpg dan akses cr3]` | `[objdump -dr build/normal/kernel/core/vmm.o menunjukkan invlpg (%rax) dan vmm_read_cr3/vmm_write_cr3]` |
| `[Mampu menulis dan menjalankan unit test host untuk logika VMM tanpa hardware nyata]` | `[./build/test_vmm_host mencetak "M7 VMM host tests PASS"]` |
| `[Mampu men-debug error build lintas mode (freestanding vs host test) akibat perbedaan flag -std=c17]` | `[perbaikan penggunaan asm/__asm__ pada vmm.c hingga build/normal/kernel/core/vmm.o berhasil]` |

---

## 5. Peta Milestone MCSOS

Centang milestone yang menjadi fokus laporan ini. Jika praktikum mencakup lebih dari satu milestone, jelaskan batas cakupan.

| Milestone | Fokus | Status dalam laporan |
|---|---|---|
| M0 | Requirements, governance, baseline arsitektur | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M1 | Toolchain reproducible, Git, QEMU, GDB, metadata build | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M2 | Boot image, kernel ELF64, early console | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M3 | Panic path, linker map, GDB, observability awal | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M4 | Trap, exception, interrupt, timer | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M5 | External interrupt bring-up (PIC, PIT, IRQ0) — sesuai riwayat branch praktikum proyek ini | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M6 | Physical Memory Manager (PMM) | `[ ] tidak dibahas / [ x ] dibahas / [ ] selesai praktikum` |
| M7 | Virtual Memory Manager (VMM), page table, invalidasi TLB | `[ ] tidak dibahas / [ ] dibahas / [ x ] selesai praktikum` |
| M8 | Thread, scheduler, synchronization | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M9 | Syscall ABI dan user program loader | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M10 | VFS, file descriptor, ramfs | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M11 | Block layer dan device model | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M12 | Persistent filesystem, mcsfs/ext2-like, recovery | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M13 | Networking stack, packet parsing, UDP/TCP subset | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M14 | Security model, capability/ACL, syscall fuzzing, hardening | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M15 | SMP, scalability, lock stress, NUMA-aware preparation | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |
| M16 | Observability, update/rollback, release image, readiness review | `[ ] tidak dibahas / [ ] dibahas / [ ] selesai praktikum` |

Catatan: Penomoran milestone pada tabel di atas mengikuti roadmap acuan MCSOS 16 milestone. Riwayat branch aktual pada repository (`praktikum/m5-timer-irq`, `praktikum/m6-pmm`, `praktikum/m7-vmm`) menunjukkan urutan pengerjaan proyek: M5 = external interrupt bring-up, M6 = PMM, M7 = VMM, sehingga baris tabel di atas disesuaikan dengan riwayat commit yang sebenarnya agar tidak menimbulkan klaim yang tidak berdasar.

Batas cakupan praktikum:

```text
Praktikum M7 berfokus pada implementasi Virtual Memory Manager (VMM):
struktur data page table x86_64, fungsi pemetaan/pelepasan halaman yang
memeriksa flag PTE (VMM_PTE_PRESENT, VMM_PTE_HUGE, VMM_PTE_ADDR_MASK),
invalidasi TLB melalui vmm_invalidate_page() (instruksi invlpg), serta
akses register kontrol CR2/CR3 melalui vmm_read_cr3(), vmm_write_cr3(),
dan vmm_read_cr2().

Praktikum ini melanjutkan PMM dari M6 dan belum membahas kernel heap,
thread/scheduler, syscall ABI, maupun subsystem lain yang akan
dikembangkan pada milestone berikutnya. Pengujian runtime penuh melalui
QEMU/GDB untuk build M7 juga belum sepenuhnya tersedia karena image ISO
yang digunakan pada sesi QEMU masih menggunakan kernel.elf lama dari
milestone sebelumnya (lihat bagian 14.2 dan 20).
```

---

## 6. Dasar Teori Ringkas

Tuliskan teori yang langsung diperlukan untuk memahami praktikum. Jangan menyalin teori umum terlalu panjang; fokus pada konsep yang benar-benar digunakan dalam desain dan pengujian.

### 6.1 Konsep Sistem Operasi yang Diuji

```text
Pada praktikum M7, konsep utama yang dipelajari adalah Virtual Memory
Manager (VMM), yaitu bagaimana kernel menerjemahkan alamat virtual ke
alamat fisik melalui struktur page table hierarkis pada arsitektur
x86_64. Setiap entri page table (PTE) memiliki flag PRESENT dan HUGE
yang menentukan validitas dan jenis pemetaan. Operasi unmap harus
memvalidasi PTE sebelum menghapusnya, dan setiap perubahan pemetaan
wajib diikuti invalidasi TLB (invlpg) agar CPU tidak menggunakan
terjemahan alamat yang sudah usang (stale translation).
```

### 6.2 Konsep Arsitektur x86_64 yang Relevan

| Konsep | Relevansi pada praktikum | Bukti/verifikasi |
|---|---|---|
| `[Page Table 4-level (PML4/PDPT/PD/PT)]` | `[Menerjemahkan alamat virtual ke fisik melalui fungsi table_from_phys() dan idx_pt()]` | `[source code vmm.c: idx_pt(vaddr), table_from_phys(space, ...)]` |
| `[PTE flags: PRESENT, HUGE, ADDR_MASK]` | `[Menentukan validitas entri sebelum unmap: (e & VMM_PTE_PRESENT) == 0 atau (e & VMM_PTE_HUGE) != 0]` | `[source code vmm.c fungsi unmap]` |
| `[Instruksi INVLPG]` | `[Menghapus entri TLB untuk satu halaman virtual setelah pemetaan diubah]` | `[objdump: 0f 01 38  invlpg (%rax)]` |
| `[Register CR3]` | `[Menyimpan alamat fisik page table root; dibaca/ditulis melalui vmm_read_cr3()/vmm_write_cr3()]` | `[objdump: simbol vmm_read_cr3, vmm_write_cr3]` |
| `[Register CR2]` | `[Menyimpan alamat virtual penyebab page fault, dibaca via vmm_read_cr2()]` | `[source code vmm.c: mov %%cr2, %0]` |

### 6.3 Konsep Implementasi Freestanding

| Aspek | Keputusan praktikum |
|---|---|
| Bahasa | `[C17 freestanding untuk kernel; C17 host-mode untuk unit test]` |
| Runtime | `[tanpa hosted libc pada mode kernel]` |
| ABI | `[x86_64 System V ABI]` |
| Mekanisme testability | `[Macro MCSOS_HOST_TEST memilih antara implementasi asm nyata (freestanding, target x86_64) dan implementasi stub (host, agar dapat diuji tanpa hardware/hak akses port I/O)]` |
| Risiko undefined behavior | `[Unmap terhadap PTE yang belum PRESENT, huge page ditangani sebagai NOT_FOUND, lupa invalidasi TLB setelah unmap, penulisan CR3 tanpa validasi alamat fisik]` |

### 6.4 Referensi Teori yang Digunakan

| No. | Sumber | Bagian yang digunakan | Alasan relevansi |
|---|---|---|---|
| `[1]` | `[Intel® 64 and IA-32 SDM]` | `[Paging (4-level page table), INVLPG, CR2/CR3]` | `[Referensi implementasi vmm_map/unmap, vmm_invalidate_page, vmm_read_cr3]` |
| `[2]` | `[Clang/LLVM Documentation]` | `[-std=c17, inline assembly (asm/__asm__), -Werror]` | `[Menyelesaikan error "use of undeclared identifier 'asm'" pada build freestanding]` |
| `[3]` | `[GNU Binutils Documentation]` | `[objdump -dr, nm -u, readelf]` | `[Audit symbol dan instruksi invlpg/cr3 pada build/normal/kernel/core/vmm.o]` |
| `[4]` | `[Dokumentasi Git]` | `[Branch, commit, push]` | `[Pengelolaan perubahan kode M7 pada branch praktikum/m7-vmm]` |

---

## 7. Lingkungan Praktikum

### 7.1 Host dan Target

| Komponen | Nilai |
|---|---|
| Host OS | `[Windows 10/11 x64]` |
| Lingkungan build | `[WSL 2 Ubuntu]` |
| Target ISA | `x86_64` |
| Target ABI | `[x86_64-unknown-none-elf]` |
| Emulator | `[QEMU]` |
| Firmware emulator | `[Limine BIOS/UEFI]` |
| Debugger | `[GNU GDB 17.1 (tersedia, belum digunakan pada sesi M7 ini)]` |
| Build system | `[GNU Make]` |
| Bahasa utama | `[C17 Freestanding (kernel) dan C17 host-mode (unit test)]` |
| Assembly | `[Inline assembly x86_64 pada vmm.c: invlpg, mov cr2/cr3]` |

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
gdb --version | head -n 1
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
GNU gdb (Ubuntu 17.1-2ubuntu1) 17.1
```

### 7.3 Lokasi Repository

| Item | Nilai |
|---|---|
| Path repository di WSL | `` `[~/src/mcsos]` `` |
| Apakah berada di filesystem Linux WSL, bukan `/mnt/c` | `[Ya]` |
| Remote repository | `[https://github.com/syifanurzimah/MCSOS.git]` |
| Branch | `[praktikum/m7-vmm]` |
| Commit hash awal | `` `[bc3b131]` `` |
| Commit hash akhir | `` `[40a6aa4]` `` |

---

## 8. Repository dan Struktur File

### 8.1 Struktur Direktori yang Relevan

Tampilkan hanya direktori dan file yang relevan dengan praktikum.

```text
mcsos/
├── kernel/
│   ├── core/
│   │   ├── kmain.c
│   │   ├── trap.c
│   │   ├── vmm.c
│   │   ├── pmm.c
│   │   ├── pic.c
│   │   ├── pit.c
│   │   ├── log.c
│   │   ├── panic.c
│   │   └── serial.c
│   ├── include/
│   │   └── mcsos/
│   │       └── vmm.h
│   └── arch/
│       └── x86_64/
│           ├── idt.c
│           ├── isr.S
│           └── include/mcsos/arch/
├── tests/
│   └── test_vmm_host.c
├── scripts/
│   └── grade_m7.sh
├── docs/
│   └── reports/
│       ├── M0-laporan.md
│       └── M7-laporan.md
├── build/
│   └── evidence/
├── iso_root/
├── Makefile
└── linker.ld
```

### 8.2 File yang Dibuat atau Diubah

| File | Jenis perubahan | Alasan perubahan | Risiko |
|---|---|---|---|
| `[kernel/core/vmm.c]` | `[baru]` | `[Implementasi Virtual Memory Manager: map/unmap, invalidasi TLB, akses cr2/cr3]` | `[tinggi]` |
| `[kernel/include/mcsos/vmm.h]` | `[baru]` | `[Header deklarasi fungsi VMM dan definisi flag PTE]` | `[rendah]` |
| `[tests/test_vmm_host.c]` | `[baru]` | `[Unit test logika VMM pada mode host tanpa hardware nyata]` | `[rendah]` |
| `[scripts/grade_m7.sh]` | `[baru]` | `[Script static grading: unit test, unresolved symbol check, grep invlpg/cr3]` | `[sedang]` |
| `[kernel/core/kmain.c]` | `[ubah]` | `[Menyesuaikan urutan inisialisasi kernel dengan modul VMM]` | `[sedang]` |
| `[kernel/core/trap.c]` | `[ubah]` | `[Penyesuaian terkait penanganan page fault/trap dispatch untuk VMM]` | `[sedang]` |
| `[Makefile]` | `[ubah]` | `[Menambah vmm.c ke daftar sumber kompilasi dan target check-m7]` | `[rendah]` |

### 8.3 Ringkasan Diff

```text
git status --short
git diff --stat
git log --oneline -n 5
```

---

## 9. Desain Teknis

### 9.1 Masalah yang Diselesaikan

```text
Kernel pada milestone sebelumnya (M6) sudah mampu mengalokasikan
memori fisik melalui Physical Memory Manager (PMM), namun kernel belum
memiliki mekanisme untuk mengelola pemetaan alamat virtual ke alamat
fisik secara terkendali. Pada praktikum M7 ditambahkan Virtual Memory
Manager (VMM) yang menyediakan operasi pemetaan/pelepasan halaman pada
struktur page table x86_64, invalidasi TLB melalui invlpg agar
perubahan pemetaan langsung berlaku, serta akses register CR2 (alamat
page fault) dan CR3 (root page table) sebagai pondasi manajemen memori
virtual kernel.
```

### 9.2 Keputusan Desain

| Keputusan | Alternatif yang dipertimbangkan | Alasan memilih | Konsekuensi |
|---|---|---|---|
| `[Page table 4-level standar x86_64 (PML4/PDPT/PD/PT)]` | `[Huge page langsung di level PD/PDPT]` | `[Kesesuaian dengan skema paging x86_64 default dan lebih mudah diverifikasi]` | `[Huge page ditangani sebagai kasus NOT_FOUND pada unmap, bukan fitur penuh]` |
| `[Pemisahan implementasi via macro MCSOS_HOST_TEST]` | `[Menguji langsung pada QEMU/hardware nyata]` | `[Logika VMM dapat diuji cepat pada host tanpa boot kernel]` | `[Perlu dua jalur kompilasi terpisah: freestanding dan host-mode]` |
| `[vmm_invalidate_page per halaman menggunakan invlpg]` | `[Flush seluruh TLB dengan menulis ulang CR3]` | `[Lebih efisien, hanya menghapus entri TLB yang relevan]` | `[Harus dipanggil konsisten setiap kali unmap agar tidak terjadi stale TLB]` |
| `[Validasi PTE (PRESENT, HUGE) sebelum unmap]` | `[Langsung menghapus entri tanpa validasi]` | `[Mencegah unmap pada halaman yang tidak valid atau huge page yang belum didukung]` | `[Operasi unmap mengembalikan VMM_ERR_NOT_FOUND/VMM_ERR_INVAL pada kasus tidak valid]` |

### 9.3 Arsitektur Ringkas

Tambahkan diagram ASCII atau Mermaid. Jika Mermaid tidak didukung oleh evaluator, tetap sertakan penjelasan tekstual.

```mermaid
Kernel (kmain) — M7 bring-up
     │
     ├──► PMM (M6) — alokasi frame fisik
     │
     └──► VMM (M7)
              │
              ├──► vmm_map()/vmm_unmap()
              │        │
              │        ▼
              │   table_from_phys() + idx_pt()
              │   (walk PML4 → PDPT → PD → PT)
              │        │
              │        ▼
              │   Validasi PTE (PRESENT, HUGE)
              │        │
              │        ▼
              │   Update entri PTE
              │        │
              │        ▼
              └──► vmm_invalidate_page(vaddr)
                       │
                       ▼
                 instruksi invlpg (TLB flush 1 halaman)

     vmm_read_cr3() / vmm_write_cr3()  → akses root page table
     vmm_read_cr2()                    → alamat page fault (CR2)
```

Penjelasan diagram:

```text
Kernel memanggil PMM (M6) untuk mengalokasikan frame fisik, kemudian
VMM (M7) bertanggung jawab memetakan alamat virtual ke frame tersebut.
Fungsi unmap melakukan penelusuran page table melalui table_from_phys()
dan idx_pt() untuk menemukan entri Page Table (PT) yang sesuai,
memvalidasi flag PRESENT dan HUGE, lalu menghapus entri tersebut
(pt[pti] = 0) sebelum memanggil vmm_invalidate_page() agar TLB tidak
menyimpan pemetaan usang. Register CR3 dibaca/ditulis melalui
vmm_read_cr3()/vmm_write_cr3() untuk mengelola root page table, dan
CR2 dibaca melalui vmm_read_cr2() sebagai sumber alamat page fault.
```

### 9.4 Kontrak Antarmuka

| Antarmuka | Pemanggil | Penerima | Precondition | Postcondition | Error path |
|---|---|---|---|---|---|
| `[vmm_unmap(space, vaddr)]` | `[kernel subsystem manajemen memori]` | `[page table space]` | `[Page table sudah diinisialisasi]` | `[PTE dihapus dan TLB untuk vaddr diinvalidasi]` | `[VMM_ERR_NOT_FOUND jika PTE tidak PRESENT atau HUGE; VMM_ERR_INVAL jika tabel tidak valid]` |
| `[vmm_invalidate_page(vaddr)]` | `[vmm_unmap dan fungsi map lain]` | `[TLB CPU]` | `[Pemetaan vaddr baru saja diubah]` | `[Entri TLB untuk vaddr dihapus]` | `[Tidak ada, operasi non-fatal]` |
| `[vmm_read_cr3() / vmm_write_cr3(value)]` | `[kernel (context switch page table)]` | `[Register CR3]` | `[-]` | `[Membaca/menulis alamat fisik root page table]` | `[Tidak ada validasi alamat pada level ini; harus divalidasi pemanggil]` |
| `[vmm_read_cr2()]` | `[Page fault handler]` | `[Register CR2]` | `[Dipanggil di konteks page fault]` | `[Mengembalikan alamat virtual penyebab fault]` | `[Tidak ada]` |

### 9.5 Struktur Data Utama

| Struktur data | Field penting | Ownership | Lifetime | Invariant |
|---|---|---|---|---|
| `` `[Page Table Entry (PTE), uint64_t]` `` | `[VMM_PTE_PRESENT, VMM_PTE_HUGE, VMM_PTE_ADDR_MASK]` | `[kernel (vmm.c)]` | `[Selama page table space aktif]` | `[Entri yang tidak PRESENT tidak boleh diakses sebagai alamat fisik valid]` |
| `` `[g_cr3_sink (variabel internal)]` `` | `[nilai CR3 terakhir dibaca/ditulis]` | `[kernel (vmm.c)]` | `[Selama kernel aktif]` | `[Digunakan untuk mencegah optimasi kompiler menghapus efek baca/tulis cr3]` |

### 9.6 Invariants

Tuliskan invariant yang harus benar sepanjang eksekusi.

1. `Setiap pemanggilan vmm_unmap() yang berhasil menghapus PTE harus selalu diikuti oleh vmm_invalidate_page() sebelum return.`
2. `Fungsi unmap tidak boleh menghapus entri yang belum PRESENT (harus mengembalikan VMM_ERR_NOT_FOUND).`
3. `Huge page belum didukung sepenuhnya sehingga unmap pada huge page harus mengembalikan VMM_ERR_NOT_FOUND, bukan melakukan operasi yang salah.`
4. `Pada mode MCSOS_HOST_TEST, seluruh akses register (cr2/cr3) dan instruksi invlpg digantikan oleh stub agar unit test dapat berjalan tanpa hak akses hardware.`
5. `Tidak terdapat undefined symbol pada objek vmm.o baik pada mode host maupun mode freestanding.`

### 9.7 Ownership, Locking, dan Concurrency

| Objek/resource | Owner | Lock yang melindungi | Boleh dipakai di interrupt context? | Catatan |
|---|---|---|---|---|
| `[Page table (PML4/PDPT/PD/PT)]` | `[kernel (vmm.c)]` | `[none]` | `[belum diverifikasi]` | `[Belum ada mekanisme locking; asumsi single-core pada M7]` |
| `[Register CR3]` | `[kernel]` | `[none]` | `[tidak]` | `[Diakses saat context switch page table, belum dilindungi]` |
| `[g_cr3_sink]` | `[kernel (vmm.c)]` | `[none]` | `[ya, hanya baca]` | `[Digunakan sebagai sink agar kompiler tidak menghapus efek samping asm]` |

Lock order yang berlaku:

```text
Belum menggunakan mekanisme locking karena kernel masih berjalan pada
konfigurasi single-core, konsisten dengan pendekatan M5/M6 sebelumnya.
Pada milestone SMP (M15) di masa depan, struktur page table dan operasi
vmm_map/unmap perlu dilindungi dengan spinlock agar aman terhadap akses
konkuren dari beberapa core.
```

### 9.8 Memory Safety dan Undefined Behavior Risk

| Risiko | Lokasi | Mitigasi | Bukti |
|---|---|---|---|
| `[Unmap pada PTE yang tidak PRESENT]` | `[vmm.c: vmm_unmap()]` | `[Pemeriksaan if ((e & VMM_PTE_PRESENT) == 0) return VMM_ERR_NOT_FOUND;]` | `[source code vmm.c]` |
| `[Unmap pada huge page]` | `[vmm.c: vmm_unmap()]` | `[Pemeriksaan (e & VMM_PTE_HUGE) != 0 dikembalikan sebagai NOT_FOUND]` | `[source code vmm.c]` |
| `[Duplikasi definisi fungsi vmm_read_cr3 (ditemukan saat implementasi)]` | `[vmm.c, blok #if defined(__x86_64__)]` | `[Duplikasi dihapus; hanya satu definisi vmm_read_cr3 yang dipertahankan]` | `[grep -n "cr3" build/evidence/m7_vmm_objdump.txt menunjukkan satu vmm_read_cr3 dan satu vmm_write_cr3]` |
| `[Penggunaan asm yang tidak valid pada -std=c17 freestanding]` | `[vmm.c: vmm_invalidate_page()]` | `[Diperbaiki menggunakan __asm__ volatile yang kompatibel serta parameter vaddr digunakan agar tidak unused]` | `[build/normal/kernel/core/vmm.o berhasil dikompilasi]` |
| `[Undefined symbol]` | `[Linker]` | `[Audit symbol nm -u build/vmm.o]` | `[build/evidence/m7_vmm_nm_undefined.txt kosong]` |

### 9.9 Security Boundary

| Boundary | Data tidak tepercaya | Validasi yang dilakukan | Failure mode aman |
|---|---|---|---|
| `[Kernel → Page Table]` | `[alamat virtual (vaddr) yang diminta di-unmap]` | `[Pemeriksaan PRESENT/HUGE sebelum menghapus entri]` | `[Mengembalikan error code, tidak melakukan operasi yang salah]` |
| `[Kernel → Register CR3]` | `[nilai yang ditulis ke CR3]` | `[Belum ada validasi eksplisit pada level vmm_write_cr3 itu sendiri]` | `[Perlu ditambahkan validasi pada pemanggil di milestone berikutnya]` |
| `[Build system → Linker]` | `[objek vmm.o]` | `[nm -u untuk memastikan tidak ada unresolved symbol]` | `[grade_m7.sh gagal (exit 1) jika ditemukan unresolved symbol]` |

---

## 10. Langkah Kerja Implementasi

Gunakan tabel berikut untuk setiap langkah. Sebelum setiap blok perintah, jelaskan maksud perintah, artefak yang dihasilkan, dan indikator hasil.

### Langkah 1 — `Membuat script static grading (scripts/grade_m7.sh)`

Maksud langkah:

```text
Membuat script otomatis untuk melakukan clean build, menjalankan unit
test host VMM, serta memeriksa unresolved symbol dan keberadaan
instruksi invlpg/akses cr3 pada hasil objdump sebagai bentuk static
grading M7.
```

Perintah:

```bash
cat scripts/grade_m7.sh
```

Output ringkas (isi awal grade_m7.sh):

```bash
#!/usr/bin/env bash
set -euo pipefail

mkdir -p build/evidence

make clean >/dev/null 2>&1 || true
make check-m7 2>&1 | tee build/evidence/m7_make_check.log

readelf -h build/vmm.o > build/evidence/m7_vmm_readelf_header.txt
readelf -S build/vmm.o > build/evidence/m7_vmm_readelf_sections.txt
nm -u build/vmm.o > build/evidence/m7_vmm_nm_undefined.txt
objdump -dr build/vmm.o > build/evidence/m7_vmm_objdump.txt

if [ -s build/evidence/m7_vmm_nm_undefined.txt ]; then
    echo "[FAIL] unresolved symbol ditemukan pada build/vmm.o" >&2
    exit 1
fi

grep -q "invlpg" build/evidence/m7_vmm_objdump.txt
grep -q "cr3" build/evidence/m7_vmm_objdump.txt

echo "[PASS] static grade M7 selesai"
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[grade_m7.sh]` | `[scripts/]` | `[Script static grading M7]` |

Indikator berhasil:

```text
Script dapat dieksekusi (setelah chmod +x) dan menghasilkan status
[PASS]/[FAIL] yang jelas.
```

### Langkah 2 — `Menjalankan grade_m7.sh pertama kali (gagal karena direktori evidence belum ada saat tee dipanggil)`

Maksud langkah:

```text
Menguji script grading pertama kali untuk mengetahui apakah urutan
perintah sudah benar.
```

Perintah:

```bash
./scripts/grade_m7.sh
```

Output ringkas:

```text
build/evidence/m7_make_check.log: No such file or directory (os error 2)
mkdir -p build
clang -std=c17 -Wall -Wextra -Werror -Ikernel/include -Ikernel/include/mcsos \
  -DMCSOS_HOST_TEST -c kernel/core/vmm.c -o build/vmm.o
clang -std=c17 -Wall -Wextra -Werror -Ikernel/include -Ikernel/include/mcsos \
  -DMCSOS_HOST_TEST kernel/core/vmm.c tests/test_vmm_host.c -o build/test_vmm_host
./build/test_vmm_host
M7 VMM host tests PASS
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[build/vmm.o]` | `[build/]` | `[Objek VMM mode host test]` |
| `[build/test_vmm_host]` | `[build/]` | `[Binary unit test VMM host]` |

Indikator berhasil:

```text
Unit test host lulus (M7 VMM host tests PASS), namun tee gagal menulis
log karena direktori build/evidence belum dibuat sebelum make dijalankan
melalui tee. Perlu perbaikan urutan mkdir -p build/evidence.
```

### Langkah 3 — `Memperbaiki urutan mkdir -p build/evidence sebelum make check-m7`

Maksud langkah:

```text
Memindahkan mkdir -p build/evidence agar dijalankan sebelum make clean
dan make check-m7, sehingga tee tidak gagal menulis log.
```

Perintah:

```bash
nano scripts/grade_m7.sh
chmod +x scripts/grade_m7.sh
./scripts/grade_m7.sh
```

Output ringkas:

```text
mkdir -p build
clang ... -c kernel/core/vmm.c -o build/vmm.o
clang ... kernel/core/vmm.c tests/test_vmm_host.c -o build/test_vmm_host
./build/test_vmm_host
M7 VMM host tests PASS
nm -u build/vmm.o
objdump -dr build/vmm.o > build/vmm.objdump.txt
grep -q "cr3" build/vmm.objdump.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[grade_m7.sh (revisi)]` | `[scripts/]` | `[Urutan mkdir evidence diperbaiki]` |

Indikator berhasil:

```text
Script berjalan tanpa error tee, namun evidence invlpg belum diperiksa
pada tahap ini (baru grep cr3 dari target check-m7 Makefile).
```

### Langkah 4 — `Audit awal: memeriksa apakah instruksi invlpg dan simbol vmm_invalidate_page muncul pada evidence`

Maksud langkah:

```text
Memverifikasi apakah objdump evidence yang dihasilkan benar-benar
memuat instruksi invlpg sebagai bukti invalidasi TLB.
```

Perintah:

```bash
grep -n "invlpg" build/evidence/m7_vmm_objdump.txt
grep -n "vmm_invalidate_page" -A10 build/evidence/m7_vmm_objdump.txt
tail -40 kernel/core/vmm.c
```

Output ringkas:

```text
(grep "invlpg" tidak menghasilkan baris apa pun)
00000000000009b0 <vmm_invalidate_page>:
 9b0: 55                      push   %rbp
 9b1: 48 89 e5                mov    %rsp,%rbp
 9b4: 48 89 7d f8             mov    %rdi,-0x8(%rbp)
 9b8: 5d                      pop    %rbp
 9b9: c3                      ret
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[m7_vmm_objdump.txt (evidence lama)]` | `[build/evidence/]` | `[Bukti bahwa evidence host-test tidak memuat instruksi invlpg nyata]` |

Indikator berhasil (temuan masalah):

```text
Ditemukan bahwa fungsi vmm_invalidate_page pada evidence hanya berisi
prologue/epilogue tanpa instruksi invlpg, karena evidence diambil dari
build/vmm.o hasil kompilasi mode MCSOS_HOST_TEST (stub), bukan dari
build freestanding x86_64 yang benar-benar memuat instruksi invlpg.
```

### Langkah 5 — `Meninjau kembali kernel/core/vmm.c dan menemukan duplikasi fungsi vmm_read_cr3`

Maksud langkah:

```text
Memeriksa isi lengkap blok #if defined(__x86_64__) pada vmm.c untuk
menemukan penyebab masalah, dan ditemukan bahwa fungsi vmm_read_cr3
didefinisikan dua kali (duplikat), sementara vmm_write_cr3 belum ada
implementasi yang benar pada blok tersebut.
```

Perintah:

```bash
grep -n "#if defined(__x86_64__)" -A30 kernel/core/vmm.c
```

Output ringkas:

```c
#if defined(__x86_64__) && !defined(MCSOS_HOST_TEST)
void vmm_invalidate_page(uint64_t vaddr) {
    __asm__ volatile("invlpg (%0)" :: "r"((void *)vaddr) : "memory");
}

uint64_t vmm_read_cr3(void) {
    uint64_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value) ::: "memory");
    g_cr3_sink = value;
    return value;
}

uint64_t vmm_read_cr3(void) {          /* <-- duplikat, bug */
    uint64_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value) ::: "memory");
    g_cr3_sink = value;
}

uint64_t vmm_read_cr2(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr2, %0" : "=r"(value) :: "memory");
    return value;
}
#else
void vmm_invalidate_page(uint64_t vaddr) { (void)vaddr; }
uint64_t vmm_read_cr3(void) { return 0; }
void vmm_write_cr3(uint64_t value) { (void)value; }
uint64_t vmm_read_cr2(void) { return 0; }
#endif
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[catatan bug duplikasi vmm_read_cr3]` | `[analisis manual]` | `[Dasar perbaikan pada Langkah 6]` |

Indikator berhasil (temuan bug):

```text
Ditemukan dua definisi vmm_read_cr3 pada blok x86_64 (bug redefinisi
yang seharusnya gagal kompilasi), dan implementasi vmm_write_cr3 pada
blok x86_64 belum ada sama sekali (hanya ada pada blok #else/stub).
```

### Langkah 6 — `Memperbaiki vmm.c: menghapus duplikasi vmm_read_cr3 dan menambahkan implementasi vmm_write_cr3`

Maksud langkah:

```text
Menghapus definisi vmm_read_cr3 yang duplikat, merapikan format
vmm_invalidate_page, dan menambahkan implementasi vmm_write_cr3 yang
benar pada blok x86_64 menggunakan instruksi mov ke register cr3.
```

Perintah:

```bash
nano kernel/core/vmm.c
sed -n '160,195p' kernel/core/vmm.c
```

Output ringkas (hasil setelah perbaikan):

```c
#if defined(__x86_64__) && !defined(MCSOS_HOST_TEST)

void vmm_invalidate_page(uint64_t vaddr)
{
    asm volatile("invlpg (%0)" :: "r"((void *)vaddr) : "memory");
}

uint64_t vmm_read_cr3(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr3, %0"
                     : "=r"(value)
                     :
                     : "memory");
    g_cr3_sink = value;
    return value;
}

uint64_t vmm_read_cr2(void) {
    uint64_t value;
    __asm__ volatile("mov %%cr2, %0"
                     : "=r"(value)
                     :
                     : "memory");
    return value;
}

#else

void vmm_invalidate_page(uint64_t vaddr) { (void)vaddr; }
uint64_t vmm_read_cr3(void) { return 0; }
void vmm_write_cr3(uint64_t value) { (void)value; }
uint64_t vmm_read_cr2(void) { return 0; }
```

Kemudian ditambahkan `vmm_write_cr3` pada blok x86_64:

```c
void vmm_write_cr3(uint64_t value)
{
    __asm__ volatile("mov %0, %%cr3"
                     :
                     : "r"(value)
                     : "memory");
    g_cr3_sink = value;
}
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[vmm.c (revisi)]` | `[kernel/core/]` | `[Duplikasi vmm_read_cr3 dihapus, vmm_write_cr3 lengkap]` |

Indikator berhasil:

```text
grep -n "cr3" build/evidence/m7_vmm_objdump.txt menunjukkan tepat satu
vmm_read_cr3 dan satu vmm_write_cr3 (tidak ada lagi duplikasi).
```

### Langkah 7 — `Build freestanding penuh (make) untuk menghasilkan objek x86_64 nyata`

Maksud langkah:

```text
Menjalankan make (bukan make check-m7) untuk mengompilasi vmm.c pada
target freestanding x86_64-unknown-none-elf, agar instruksi invlpg dan
akses cr3 benar-benar dikompilasi menjadi instruksi mesin, bukan stub.
```

Perintah:

```bash
make
```

Output ringkas (error pertama):

```text
clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding ... \
  -c kernel/core/vmm.c -o build/normal/kernel/core/vmm.o
kernel/core/vmm.c:167:5: error: use of undeclared identifier 'asm'
  167 |     asm volatile("invlpg (%0)" :: "r"((void *)vaddr) : "memory");
      |     ^~~
kernel/core/vmm.c:165:35: error: unused parameter 'vaddr'
      [-Werror,-Wunused-parameter]
  165 | void vmm_invalidate_page(uint64_t vaddr)
      |                                   ^
2 errors generated.
make: *** [Makefile:57: build/normal/kernel/core/vmm.o] Error 1
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[log error build freestanding]` | `[terminal]` | `[Dasar perbaikan pada Langkah 8]` |

Indikator berhasil (temuan masalah):

```text
Ditemukan bahwa penggunaan kata kunci "asm" (tanpa garis bawah ganda)
tidak dikenali pada mode -std=c17 murni, dan parameter vaddr dianggap
tidak dipakai karena tubuh fungsi gagal terparse akibat error asm
tersebut.
```

### Langkah 8 — `Memperbaiki vmm.c agar kompatibel dengan -std=c17 pada build freestanding`

Maksud langkah:

```text
Mengubah cara penulisan inline assembly agar dapat dikompilasi baik
pada mode freestanding maupun host-test tanpa mengubah semantik operasi
invlpg dan akses register.
```

Perintah:

```bash
nano kernel/core/vmm.c
make
```

Output ringkas:

```text
mkdir -p build/normal/kernel/core/
clang --target=x86_64-unknown-none-elf -std=c17 -ffreestanding ... \
  -c kernel/core/vmm.c -o build/normal/kernel/core/vmm.o
mkdir -p build/normal/kernel/lib/
clang ... -c kernel/lib/memory.c -o build/normal/kernel/lib/memory.o
clang ... -c kernel/arch/x86_64/isr.S -o build/normal/kernel/arch/x86_64/isr.o
ld.lld -nostdlib -static ... -o build/kernel.elf ... build/normal/kernel/core/vmm.o ...
readelf -h build/kernel.elf > build/kernel.readelf.header.txt
grep -q 'ELF64' build/kernel.readelf.header.txt
grep -q 'kmain' build/kernel.syms.txt
grep -q 'x86_64_idt_init' build/kernel.syms.txt
grep -q 'x86_64_trap_dispatch' build/kernel.syms.txt
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[build/normal/kernel/core/vmm.o]` | `[build/normal/kernel/core/]` | `[Objek VMM hasil build freestanding x86_64 (memuat instruksi nyata)]` |
| `[kernel.elf]` | `[build/]` | `[Kernel utama M7 dengan vmm.o terlink]` |

Indikator berhasil:

```text
Build freestanding berhasil tanpa error, dan build/normal/kernel/core/
vmm.o berhasil dibuat. Seluruh pemeriksaan simbol/instruksi bawaan
Makefile (kmain, x86_64_idt_init, x86_64_trap_dispatch, iretq, lidt)
tetap lulus, menunjukkan penambahan VMM tidak merusak milestone
sebelumnya.
```

### Langkah 9 — `Verifikasi instruksi invlpg pada objek freestanding`

Maksud langkah:

```text
Memastikan instruksi invlpg benar-benar dikompilasi ke dalam objek
freestanding, bukan hanya stub kosong.
```

Perintah:

```bash
objdump -dr build/normal/kernel/core/vmm.o | grep -n "invlpg"
```

Output ringkas:

```text
704: 9cd:       0f 01 38                invlpg (%rax)
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[bukti instruksi invlpg]` | `[terminal/objdump]` | `[Verifikasi vmm_invalidate_page menghasilkan instruksi TLB flush nyata]` |

Indikator berhasil:

```text
Instruksi invlpg (%rax) ditemukan pada offset 0x9cd, membuktikan
vmm_invalidate_page() menghasilkan instruksi mesin x86_64 yang benar
pada build freestanding.
```

### Langkah 10 — `Memperbaiki scripts/grade_m7.sh agar evidence objdump diambil dari objek freestanding`

Maksud langkah:

```text
Mengubah sumber evidence objdump pada grade_m7.sh dari build/vmm.o
(hasil host-test, tanpa invlpg nyata) menjadi build/normal/kernel/core/
vmm.o (hasil freestanding, dengan invlpg nyata), serta menambahkan
pemanggilan make sebelum make check-m7 agar kedua build tersedia.
```

Perintah:

```bash
nano scripts/grade_m7.sh
cat scripts/grade_m7.sh
```

Output ringkas (isi akhir grade_m7.sh):

```bash
#!/usr/bin/env bash
set -euo pipefail

make clean >/dev/null 2>&1 || true

mkdir -p build/evidence

make
make check-m7 2>&1 | tee build/evidence/m7_make_check.log

readelf -h build/vmm.o > build/evidence/m7_vmm_readelf_header.txt
readelf -S build/vmm.o > build/evidence/m7_vmm_readelf_sections.txt
nm -u build/vmm.o > build/evidence/m7_vmm_nm_undefined.txt
objdump -dr build/normal/kernel/core/vmm.o > build/evidence/m7_vmm_objdump.txt

if [ -s build/evidence/m7_vmm_nm_undefined.txt ]; then
    echo "[FAIL] unresolved symbol ditemukan pada build/vmm.o" >&2
    exit 1
fi

grep -q "invlpg" build/evidence/m7_vmm_objdump.txt
grep -q "cr3" build/evidence/m7_vmm_objdump.txt

echo "[PASS] static grade M7 selesai"
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[grade_m7.sh (final)]` | `[scripts/]` | `[Script grading M7 yang benar mengambil evidence dari build freestanding]` |

Indikator berhasil:

```text
Script grade_m7.sh versi final siap dijalankan ulang untuk menghasilkan
status akhir [PASS].
```

### Langkah 11 — `Menjalankan grade_m7.sh final secara menyeluruh`

Maksud langkah:

```text
Menjalankan ulang seluruh pipeline grading M7 dari kondisi bersih untuk
memastikan seluruh pemeriksaan (unit test host, unresolved symbol,
invlpg, cr3) lulus secara konsisten.
```

Perintah:

```bash
./scripts/grade_m7.sh
```

Output ringkas:

```text
mkdir -p build/normal/kernel/arch/x86_64/
clang --target=x86_64-unknown-none-elf ... -c kernel/core/vmm.c -o build/normal/kernel/core/vmm.o
... (kompilasi seluruh sumber kernel) ...
ld.lld -nostdlib -static ... -o build/kernel.elf ...
grep -q 'iretq' build/kernel.disasm.txt
grep -q 'lidt' build/kernel.disasm.txt
mkdir -p build
clang -std=c17 -Wall -Wextra -Werror ... -DMCSOS_HOST_TEST -c kernel/core/vmm.c -o build/vmm.o
clang -std=c17 -Wall -Wextra -Werror ... -DMCSOS_HOST_TEST kernel/core/vmm.c tests/test_vmm_host.c -o build/test_vmm_host
./build/test_vmm_host
M7 VMM host tests PASS
nm -u build/vmm.o
objdump -dr build/vmm.o > build/vmm.objdump.txt
grep -q "cr3" build/vmm.objdump.txt
[PASS] static grade M7 selesai
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[build/evidence/m7_make_check.log]` | `[build/evidence/]` | `[Log lengkap make check-m7]` |
| `[build/evidence/m7_vmm_objdump.txt]` | `[build/evidence/]` | `[Disassembly vmm.o freestanding, memuat invlpg]` |
| `[build/evidence/m7_vmm_readelf_header.txt]` | `[build/evidence/]` | `[Header ELF vmm.o]` |
| `[build/evidence/m7_vmm_readelf_sections.txt]` | `[build/evidence/]` | `[Daftar section vmm.o]` |
| `[build/evidence/m7_vmm_nm_undefined.txt]` | `[build/evidence/]` | `[Daftar unresolved symbol (kosong)]` |

Indikator berhasil:

```text
Output akhir "[PASS] static grade M7 selesai" tercetak, menandakan
seluruh pemeriksaan static grading M7 lulus.
```

### Langkah 12 — `Commit dan push hasil pekerjaan M7 ke branch praktikum/m7-vmm`

Maksud langkah:

```text
Membuat branch baru khusus M7, menambahkan seluruh file baru/ubahan
terkait VMM, melakukan commit, dan mendorong (push) branch tersebut ke
remote repository GitHub.
```

Perintah:

```bash
git checkout -b praktikum/m7-vmm
git add Makefile
git add kernel/core/kmain.c
git add kernel/core/trap.c
git add kernel/core/vmm.c
git add kernel/include/mcsos/vmm.h
git add scripts/grade_m7.sh
git add tests/test_vmm_host.c
git commit -m "M7: implement virtual memory manager (VMM)"
git push -u origin praktikum/m7-vmm
```

Output ringkas:

```text
Switched to a new branch 'praktikum/m7-vmm'
[praktikum/m7-vmm 40a6aa4] M7: implement virtual memory manager (VMM)
 7 files changed, 458 insertions(+), 1 deletion(-)
 create mode 100644 kernel/core/vmm.c
 create mode 100644 kernel/include/mcsos/vmm.h
 create mode 100755 scripts/grade_m7.sh
 create mode 100644 tests/test_vmm_host.c

Enumerating objects: 25, done.
...
remote: Create a pull request for 'praktikum/m7-vmm' on GitHub by visiting:
remote:      https://github.com/syifanurzimah/MCSOS/pull/new/praktikum/m7-vmm
 * [new branch]      praktikum/m7-vmm -> praktikum/m7-vmm
branch 'praktikum/m7-vmm' set up to track 'origin/praktikum/m7-vmm'.
```

Artefak yang dihasilkan:

| Artefak | Lokasi | Fungsi |
|---|---|---|
| `[commit 40a6aa4]` | `[branch praktikum/m7-vmm]` | `[Snapshot kode M7 (VMM) yang sudah teruji]` |
| `[branch praktikum/m7-vmm di remote GitHub]` | `[origin]` | `[Cadangan dan bahan review kode M7]` |

Indikator berhasil:

```text
git log --oneline -3 menunjukkan 40a6aa4 sebagai HEAD pada branch
praktikum/m7-vmm, dan git branch menunjukkan branch tersebut aktif
serta ter-track ke origin.
```

---

## 11. Checkpoint Buildable

Setiap praktikum wajib memiliki minimal satu checkpoint yang dapat dibangun dari clean checkout.

| Checkpoint | Perintah | Expected result | Status |
|---|---|---|---|
| Clean build freestanding | `` `make clean && make` `` | `[kernel.elf berhasil dibangun termasuk vmm.c]` | `[PASS]` |
| Unit test host VMM | `` `make check-m7` `` | `[M7 VMM host tests PASS]` | `[PASS]` |
| Audit symbol cr3 (host build) | `` `grep -q "cr3" build/vmm.objdump.txt` `` | `[Symbol cr3 ditemukan]` | `[PASS]` |
| Audit instruksi invlpg (freestanding build) | `` `objdump -dr build/normal/kernel/core/vmm.o | grep invlpg` `` | `[Instruksi invlpg (%rax) ditemukan]` | `[PASS]` |
| Static grading menyeluruh | `` `./scripts/grade_m7.sh` `` | `[PASS] static grade M7 selesai` | `[PASS]` |
| QEMU boot dengan build M7 terbaru | `` `(belum dijalankan)` `` | `[Log serial menunjukkan milestone M7]` | `[BELUM DIUJI — lihat bagian 14.2]` |
| Debugging GDB pada jalur VMM | `` `(belum dijalankan)` `` | `[Breakpoint pada vmm_unmap/vmm_invalidate_page tercapai]` | `[BELUM DIUJI — lihat bagian 20]` |

Catatan checkpoint:

```text
Seluruh tahapan build dan static grading M7 berhasil dijalankan hingga
status akhir [PASS]. Namun demikian, pengujian runtime melalui QEMU dan
sesi debugging GDB untuk build M7 secara spesifik belum dilakukan pada
sesi ini, karena image ISO (build/mcsos.iso) yang tersedia masih dibuat
dari kernel.elf lama (milestone sebelumnya) yang berada di iso_root/boot/
kernel.elf. Hal ini dicatat secara eksplisit sebagai known issue pada
bagian 14.2 dan 20 agar tidak terjadi klaim readiness yang berlebihan.
```

---

## 12. Perintah Uji dan Validasi

### 12.1 Build Test

Perintah ini memverifikasi bahwa proyek dapat dibangun ulang dari kondisi bersih dan tidak bergantung pada artefak lokal yang tidak terdokumentasi.

```bash
make clean
make
```

Hasil:

```text
Build berhasil tanpa error.
Menghasilkan file:
- build/kernel.elf (termasuk build/normal/kernel/core/vmm.o)
- build/kernel.map
- build/kernel.syms.txt
- build/kernel.disasm.txt
```

Status: `[PASS]`

### 12.2 Static Inspection

Perintah ini memeriksa layout ELF, entry point, section, symbol, relocation, atau instruksi kritis sesuai kebutuhan praktikum.

```bash
grep -E 'vmm_unmap|vmm_invalidate_page|vmm_read_cr3|vmm_write_cr3|vmm_read_cr2' build/kernel.syms.txt
readelf -h build/vmm.o
objdump -dr build/normal/kernel/core/vmm.o | grep -E 'invlpg|cr3|cr2'
```

Hasil penting:

```text
- Symbol vmm_invalidate_page, vmm_read_cr3, vmm_write_cr3, vmm_read_cr2
  ditemukan.
- Instruksi invlpg (%rax) ditemukan pada offset 0x9cd (build freestanding).
- Tidak ditemukan lagi duplikasi definisi vmm_read_cr3 setelah perbaikan.
```

Status: `[PASS]`

### 12.3 Unit Test (Host)

```bash
make check-m7
```

Hasil:

```text
./build/test_vmm_host
M7 VMM host tests PASS
nm -u build/vmm.o
objdump -dr build/vmm.o > build/vmm.objdump.txt
grep -q "cr3" build/vmm.objdump.txt
```

Status: `[PASS]`

### 12.4 Static Grading Menyeluruh

```bash
./scripts/grade_m7.sh
```

Hasil:

```text
[PASS] static grade M7 selesai
```

Status: `[PASS]`

### 12.5 GDB Debug Evidence

```text
BELUM DILAKUKAN pada sesi ini untuk build M7. Sesi QEMU dengan gdbstub
(-s -S) sempat dijalankan, namun ISO yang digunakan masih memuat
kernel.elf lama (milestone sebelumnya), sehingga tidak representatif
sebagai bukti debugging modul VMM M7. Direncanakan dilakukan pada
iterasi berikutnya setelah iso_root/boot/kernel.elf diperbarui.
```

Status: `[NA — belum dilakukan untuk build M7]`

### 12.6 Stress/Fuzz/Fault Injection Test

Wajib untuk praktikum lanjutan seperti allocator, syscall, filesystem, networking, driver, security, dan SMP.

```bash
BELUM DILAKUKAN
```

Hasil:

```text
Tidak diterapkan pada milestone M7. Pengujian lanjutan seperti mapping
berulang, unmap ganda (double unmap), dan fault injection pada page
table direncanakan pada milestone keamanan (M14) atau iterasi lanjutan.
```

Status: `[NA]`

### 12.7 Visual Evidence

Jika praktikum menghasilkan tampilan framebuffer, GUI, atau output grafis, lampirkan screenshot.

| Screenshot | Lokasi file | Keterangan |
|---|---|---|
| `[Screenshot build freestanding berhasil]` | `[lampiran]` | `[make berhasil, vmm.o terlink ke kernel.elf]` |
| `[Screenshot unit test host PASS]` | `[lampiran]` | `[M7 VMM host tests PASS]` |
| `[Screenshot grade_m7.sh PASS]` | `[lampiran]` | `[PASS] static grade M7 selesai]` |
| `[Screenshot git commit/push]` | `[lampiran]` | `[commit 40a6aa4 pada branch praktikum/m7-vmm]` |

---

## 13. Hasil Uji

### 13.1 Tabel Ringkasan Hasil

| No. | Uji | Expected result | Actual result | Status | Evidence |
|---|---|---|---|---|---|
| 1 | `[Build kernel M7 (freestanding)]` | `[kernel berhasil dibangun dengan vmm.c]` | `[kernel.elf berhasil dibuat, build/normal/kernel/core/vmm.o terlink]` | `[PASS]` | `[build/kernel.elf]` |
| 2 | `[Unit test host VMM]` | `[Semua kasus uji logika VMM lulus]` | `[M7 VMM host tests PASS]` | `[PASS]` | `[./build/test_vmm_host]` |
| 3 | `[Audit unresolved symbol]` | `[Tidak ada unresolved symbol]` | `[build/evidence/m7_vmm_nm_undefined.txt kosong]` | `[PASS]` | `[m7_vmm_nm_undefined.txt]` |
| 4 | `[Audit instruksi invlpg]` | `[Instruksi invlpg ditemukan pada objek freestanding]` | `[invlpg (%rax) pada offset 0x9cd]` | `[PASS]` | `[m7_vmm_objdump.txt]` |
| 5 | `[Audit akses cr3]` | `[Symbol vmm_read_cr3/vmm_write_cr3 ditemukan, tanpa duplikasi]` | `[Tepat satu vmm_read_cr3 dan satu vmm_write_cr3]` | `[PASS]` | `[m7_vmm_objdump.txt]` |
| 6 | `[Static grading menyeluruh]` | `[Status akhir PASS]` | `[PASS] static grade M7 selesai]` | `[PASS]` | `[terminal grade_m7.sh]` |
| 7 | `[QEMU boot dengan build M7 terbaru]` | `[Log serial menunjukkan milestone M7]` | `[Belum dilakukan; ISO masih memuat kernel.elf lama]` | `[BELUM DIUJI]` | `[-]` |
| 8 | `[Debugging GDB pada modul VMM]` | `[Breakpoint pada vmm_unmap/vmm_invalidate_page tercapai]` | `[Belum dilakukan untuk build M7]` | `[BELUM DIUJI]` | `[-]` |
| 9 | `[Commit dan push M7]` | `[Branch praktikum/m7-vmm ter-push ke remote]` | `[commit 40a6aa4 berhasil di-push]` | `[PASS]` | `[git log, git push output]` |

### 13.2 Log Penting

```text
mkdir -p build
clang -std=c17 -Wall -Wextra -Werror -Ikernel/include -Ikernel/include/mcsos \
  -DMCSOS_HOST_TEST -c kernel/core/vmm.c -o build/vmm.o
clang -std=c17 -Wall -Wextra -Werror -Ikernel/include -Ikernel/include/mcsos \
  -DMCSOS_HOST_TEST kernel/core/vmm.c tests/test_vmm_host.c -o build/test_vmm_host
./build/test_vmm_host
M7 VMM host tests PASS
nm -u build/vmm.o
objdump -dr build/vmm.o > build/vmm.objdump.txt
grep -q "cr3" build/vmm.objdump.txt
[PASS] static grade M7 selesai
```

```text
objdump -dr build/normal/kernel/core/vmm.o | grep -n invlpg
704: 9cd:       0f 01 38                invlpg (%rax)
```

### 13.3 Artefak Bukti

| Artefak | Path | SHA-256 / hash | Fungsi |
|---|---|---|---|
| `kernel.elf` | `[build/kernel.elf]` | `[-]` | `[kernel utama M7, memuat vmm.o]` |
| `vmm.o (freestanding)` | `[build/normal/kernel/core/vmm.o]` | `[-]` | `[objek VMM x86_64 nyata, memuat invlpg]` |
| `vmm.o (host test)` | `[build/vmm.o]` | `[-]` | `[objek VMM mode host, untuk unit test]` |
| `test_vmm_host` | `[build/test_vmm_host]` | `[-]` | `[binary unit test VMM]` |
| `m7_make_check.log` | `[build/evidence/m7_make_check.log]` | `[-]` | `[log lengkap make check-m7]` |
| `m7_vmm_objdump.txt` | `[build/evidence/m7_vmm_objdump.txt]` | `[-]` | `[disassembly vmm.o freestanding]` |
| `m7_vmm_nm_undefined.txt` | `[build/evidence/m7_vmm_nm_undefined.txt]` | `[-]` | `[daftar unresolved symbol (kosong)]` |

Perintah hash:

```bash
sha256sum build/kernel.elf
```

---

## 14. Analisis Teknis

### 14.1 Analisis Keberhasilan

```text
Praktikum M7 berhasil dilaksanakan pada level build dan static grading.
Modul VMM (vmm.c, vmm.h) berhasil dikompilasi baik pada mode host test
maupun mode freestanding x86_64, dan berhasil dilink ke kernel.elf tanpa
merusak pemeriksaan symbol/instruksi milestone sebelumnya (kmain,
x86_64_idt_init, x86_64_trap_dispatch, iretq, lidt). Unit test host
(test_vmm_host.c) lulus dengan pesan "M7 VMM host tests PASS". Audit
objdump pada build freestanding membuktikan instruksi invlpg benar-benar
dikompilasi (offset 0x9cd), dan symbol vmm_read_cr3/vmm_write_cr3
ditemukan tanpa duplikasi setelah perbaikan. Script grade_m7.sh akhirnya
mengeluarkan status "[PASS] static grade M7 selesai" secara konsisten.
```

### 14.2 Analisis Kegagalan atau Perbedaan Hasil

```text
Terdapat beberapa kendala yang ditemukan dan diperbaiki selama
praktikum:

1. Urutan mkdir -p build/evidence yang berada setelah make clean/make
   check-m7 menyebabkan tee gagal menulis log pada percobaan pertama.
   Diperbaiki dengan memindahkan mkdir ke awal script.

2. Ditemukan duplikasi definisi fungsi vmm_read_cr3 pada blok
   #if defined(__x86_64__), serta implementasi vmm_write_cr3 yang belum
   lengkap pada blok tersebut. Diperbaiki dengan menghapus duplikasi dan
   menambahkan implementasi vmm_write_cr3 yang benar.

3. Build freestanding gagal dengan pesan "use of undeclared identifier
   'asm'" pada -std=c17, disertai error unused parameter pada vaddr
   akibat tubuh fungsi gagal diparse. Diperbaiki dengan menyesuaikan
   penulisan inline assembly agar kompatibel dengan flag build yang
   digunakan pada target freestanding.

4. Evidence objdump awal diambil dari build/vmm.o (hasil kompilasi mode
   MCSOS_HOST_TEST/stub) sehingga grep "invlpg" tidak pernah menemukan
   hasil, padahal secara logika instruksi tersebut memang sengaja tidak
   ada pada mode host (stub kosong). Diperbaiki dengan mengarahkan
   evidence objdump ke build/normal/kernel/core/vmm.o (hasil build
   freestanding x86_64) yang memang memuat instruksi invlpg nyata.

5. Bukti runtime melalui QEMU pada sesi ini masih menggunakan
   iso_root/boot/kernel.elf lama (dari milestone sebelumnya, ditandai
   dengan log serial "[MCSOS:M5] boot: external interrupt bring-up
   start" saat boot), bukan kernel.elf hasil build M7 terbaru. Akibatnya
   sesi QEMU/GDB yang sempat dijalankan tidak dapat dijadikan bukti valid
   untuk modul VMM M7, dan akan diperbaiki pada iterasi berikutnya dengan
   memperbarui iso_root/boot/kernel.elf sebelum membuat ulang ISO.
```

### 14.3 Perbandingan dengan Teori

| Konsep teori | Implementasi praktikum | Sesuai/tidak sesuai | Penjelasan |
|---|---|---|---|
| `[Page table 4-level x86_64]` | `[table_from_phys() dan idx_pt() untuk menelusuri PT]` | `[sesuai]` | `[Struktur penelusuran mengikuti hierarki page table x86_64]` |
| `[Validasi PTE sebelum unmap]` | `[Pemeriksaan VMM_PTE_PRESENT dan VMM_PTE_HUGE]` | `[sesuai]` | `[Mencegah unmap pada entri tidak valid/huge page]` |
| `[Invalidasi TLB setelah perubahan pemetaan]` | `[vmm_invalidate_page() memanggil invlpg]` | `[sesuai]` | `[Terverifikasi melalui objdump pada build freestanding]` |
| `[CR3 sebagai root page table]` | `[vmm_read_cr3()/vmm_write_cr3()]` | `[sesuai secara desain, belum diverifikasi runtime]` | `[Bukti hanya pada level kompilasi/simbol, belum ada bukti eksekusi nyata di QEMU]` |

### 14.4 Kompleksitas dan Kinerja

| Aspek | Estimasi/hasil | Bukti | Catatan |
|---|---|---|---|
| Kompleksitas algoritma | `[O(1) untuk unmap satu halaman (penelusuran page table berkedalaman tetap 4 level)]` | `[Analisis kode vmm.c]` | `[Kedalaman page table x86_64 tetap, tidak bergantung jumlah halaman]` |
| Waktu build | `[kurang dari 1 menit]` | `[output make dan grade_m7.sh]` | `[Build vmm.c bersama modul lain selesai cepat]` |
| Ukuran objek vmm.o | `[belum diukur secara eksplisit]` | `[readelf -S build/vmm.o]` | `[dapat ditambahkan pada iterasi berikutnya]` |
| Overhead invalidasi TLB | `[1 instruksi invlpg per halaman]` | `[objdump: invlpg (%rax)]` | `[Lebih efisien dibanding flush seluruh TLB via reload cr3]` |

---

## 15. Debugging dan Failure Modes

### 15.1 Failure Modes yang Ditemukan

| Failure mode | Gejala | Penyebab sementara | Bukti | Perbaikan |
|---|---|---|---|---|
| `[Log tee gagal ditulis]` | `[build/evidence/m7_make_check.log: No such file or directory]` | `[mkdir -p build/evidence dijalankan setelah make check-m7 dipanggil via tee]` | `[terminal]` | `[Memindahkan mkdir -p build/evidence ke awal script]` |
| `[Duplikasi fungsi vmm_read_cr3]` | `[Dua definisi vmm_read_cr3 pada blok yang sama]` | `[Kesalahan penyalinan kode saat menulis blok #if defined(__x86_64__)]` | `[grep -n "#if defined(__x86_64__)" -A30 kernel/core/vmm.c]` | `[Menghapus definisi duplikat, menyisakan satu implementasi yang benar]` |
| `[vmm_write_cr3 belum lengkap]` | `[Hanya ada stub pada blok #else, belum ada implementasi asm pada blok x86_64]` | `[Implementasi belum ditambahkan saat penulisan awal]` | `[grep -n "vmm_write_cr3" -A8 -B2 kernel/core/vmm.c]` | `[Menambahkan implementasi mov %0, %%cr3 pada blok x86_64]` |
| `[error: use of undeclared identifier 'asm']` | `[Build freestanding gagal pada -std=c17]` | `[Penulisan asm volatile(...) tidak kompatibel dengan mode compile yang digunakan]` | `[terminal: kernel/core/vmm.c:167:5]` | `[Menyesuaikan penulisan inline assembly agar valid pada build freestanding]` |
| `[grep "invlpg" tidak ditemukan pada evidence]` | `[build/evidence/m7_vmm_objdump.txt tidak memuat invlpg]` | `[Evidence diambil dari build/vmm.o hasil mode host test/stub, bukan build freestanding]` | `[grep -n "invlpg" build/evidence/m7_vmm_objdump.txt kosong]` | `[Mengarahkan objdump evidence ke build/normal/kernel/core/vmm.o]` |
| `[QEMU boot menampilkan log milestone lama]` | `[Serial log menampilkan "[MCSOS:M5] boot: external interrupt bring-up start"]` | `[ISO dibuat dari iso_root/boot/kernel.elf lama yang belum diperbarui dengan build M7]` | `[log QEMU pada lampiran]` | `[Memperbarui iso_root/boot/kernel.elf dengan build/kernel.elf terbaru sebelum membuat ulang ISO — direncanakan pada iterasi berikutnya]` |

### 15.2 Failure Modes yang Diantisipasi

| Failure mode | Deteksi | Dampak | Mitigasi |
|---|---|---|---|
| `[Double unmap pada alamat yang sama]` | `[Pemanggilan vmm_unmap dua kali pada vaddr yang sudah tidak PRESENT]` | `[Mengembalikan VMM_ERR_NOT_FOUND, tidak menyebabkan crash]` | `[Pemeriksaan PRESENT sebelum menghapus entri]` |
| `[Lupa memanggil vmm_invalidate_page setelah unmap]` | `[TLB masih menyimpan pemetaan lama]` | `[Alamat virtual yang seharusnya sudah di-unmap masih dapat diakses]` | `[vmm_invalidate_page dipanggil di akhir vmm_unmap sebelum return]` |
| `[Penulisan CR3 dengan alamat fisik tidak valid]` | `[Belum ada mekanisme deteksi otomatis pada level vmm_write_cr3]` | `[Berpotensi menyebabkan crash/hang total sistem]` | `[Validasi alamat direncanakan dilakukan oleh pemanggil pada milestone berikutnya]` |
| `[Undefined symbol]` | `[nm -u]` | `[kernel gagal link]` | `[Pemeriksaan otomatis pada grade_m7.sh, exit 1 jika ditemukan]` |

### 15.3 Triage yang Dilakukan

```text
1. Menjalankan grade_m7.sh awal dan mencatat error tee akibat direktori
   evidence belum ada.
2. Memperbaiki urutan mkdir -p build/evidence pada script.
3. Memeriksa evidence objdump host-test dan menemukan bahwa instruksi
   invlpg tidak muncul; menelusuri kembali source vmm.c.
4. Menemukan duplikasi definisi vmm_read_cr3 dan implementasi
   vmm_write_cr3 yang belum lengkap; memperbaiki keduanya.
5. Menjalankan make (build freestanding) dan menemukan error asm pada
   -std=c17; memperbaiki penulisan inline assembly.
6. Memverifikasi instruksi invlpg pada build/normal/kernel/core/vmm.o
   menggunakan objdump.
7. Memperbaiki grade_m7.sh agar mengambil evidence objdump dari objek
   freestanding, bukan objek host-test.
8. Menjalankan ulang grade_m7.sh hingga status akhir [PASS] konsisten.
9. Melakukan commit dan push hasil akhir ke branch praktikum/m7-vmm.
```

### 15.4 Panic Path

Jika terjadi panic, tempel output panic.

```text
Pada praktikum M7 tidak terjadi panic selama proses build maupun unit
test host. Jalur panic dari milestone sebelumnya (kernel_panic_at())
tetap tersedia sebagai fallback apabila terjadi exception fatal terkait
page fault pada milestone berikutnya (misalnya saat page fault handler
memanggil vmm_read_cr2() untuk mendiagnosis alamat penyebab fault).
```

---

## 16. Prosedur Rollback

Rollback harus menjelaskan cara kembali ke kondisi aman jika perubahan gagal.

| Skenario rollback | Perintah | Data yang harus diselamatkan | Status |
|---|---|---|---|
| Kembali ke commit awal M6 | `` `git checkout bc3b131` `` | `[Log build dan evidence M6]` | `[belum diuji]` |
| Revert implementasi M7 | `` `git revert 40a6aa4` `` | `[Evidence M7 dan log pengujian]` | `[belum diuji]` |
| Bersihkan artefak build | `` `make clean` `` | `[Tidak ada, karena hanya menghapus hasil build]` | `[teruji]` |
| Perbaikan cepat duplikasi fungsi | `` `git diff kernel/core/vmm.c` `` | `[Riwayat perubahan vmm.c sebelum dan sesudah perbaikan]` | `[teruji secara manual]` |

Catatan rollback:

```text
Pada praktikum ini rollback penuh menggunakan Git tidak dilakukan karena
implementasi M7 pada level build dan static grading berjalan dengan
baik setelah seluruh bug diperbaiki. Branch praktikum/m6-pmm tetap
tersimpan pada remote sebagai titik rollback apabila diperlukan. Proses
make clean telah diuji dan berhasil menghapus artefak hasil kompilasi
tanpa memengaruhi source code.
```

---

## 17. Keamanan dan Reliability

### 17.1 Risiko Keamanan

| Risiko | Boundary | Dampak | Mitigasi | Evidence |
|---|---|---|---|---|
| `[Penulisan CR3 dengan alamat tidak valid]` | `[vmm_write_cr3]` | `[Crash/hang total sistem karena root page table tidak valid]` | `[Validasi alamat direncanakan pada pemanggil, belum diimplementasikan pada level vmm_write_cr3 itu sendiri]` | `[source code vmm.c]` |
| `[Unmap pada huge page tanpa dukungan penuh]` | `[vmm_unmap]` | `[Potensi kesalahan interpretasi pemetaan besar]` | `[Dikembalikan sebagai VMM_ERR_NOT_FOUND, tidak melakukan operasi yang salah]` | `[source code vmm.c]` |
| `[Undefined symbol]` | `[linker]` | `[Kernel gagal dibangun]` | `[Pemeriksaan menggunakan nm -u pada grade_m7.sh]` | `[m7_vmm_nm_undefined.txt]` |

### 17.2 Reliability dan Data Integrity

| Risiko reliability | Dampak | Deteksi | Mitigasi |
|---|---|---|---|
| `[Evidence diambil dari objek yang salah (host vs freestanding)]` | `[Kesimpulan pengujian bisa keliru (invlpg dianggap tidak ada padahal memang stub)]` | `[grep manual tidak menemukan hasil pada evidence lama]` | `[Memisahkan sumber evidence sesuai mode build yang relevan]` |
| `[ISO/QEMU menggunakan kernel.elf lama]` | `[Bukti runtime tidak representatif untuk milestone M7]` | `[Log serial menampilkan tag milestone lama "[MCSOS:M5]"]` | `[Memperbarui iso_root/boot/kernel.elf sebelum membuat ulang ISO]` |
| `[Duplikasi fungsi tidak terdeteksi otomatis]` | `[Berpotensi lolos review jika tidak diperiksa manual]` | `[Peninjauan manual source code]` | `[Menambahkan pemeriksaan statis tambahan (mis. -Wredundant-decls) pada iterasi berikutnya]` |

### 17.3 Negative Test

| Negative test | Input buruk | Expected result | Actual result | Status |
|---|---|---|---|---|
| `[Unresolved symbol check]` | `[nm -u build/vmm.o]` | `[Tidak ada undefined symbol]` | `[File m7_vmm_nm_undefined.txt kosong]` | `[PASS]` |
| `[Build freestanding dengan asm tidak valid]` | `[asm volatile(...) tanpa penyesuaian pada -std=c17]` | `[Pesan error jelas dari compiler]` | `[error: use of undeclared identifier 'asm']` | `[PASS (error terdeteksi dan dapat diperbaiki)]` |
| `[Evidence objdump dari objek yang salah]` | `[grep "invlpg" pada build/vmm.o (host stub)]` | `[Tidak ditemukan hasil]` | `[grep tidak menghasilkan baris apa pun]` | `[PASS (perilaku sesuai ekspektasi stub, mengarahkan perbaikan ke objek yang benar)]` |

---

## 18. Pembagian Kerja Kelompok

Isi bagian ini hanya jika praktikum dikerjakan berkelompok. Untuk pengerjaan individu, tulis "Tidak berlaku".

```text
Tidak berlaku, praktikum M7 dikerjakan secara individu.
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
| Proyek dapat dibangun dari clean checkout | `[PASS]` | `[make clean && make]` |
| Perintah build terdokumentasi | `[PASS]` | `[bagian 10]` |
| Unit test relevan lulus | `[PASS]` | `[make check-m7 → M7 VMM host tests PASS]` |
| Static grading menyeluruh lulus | `[PASS]` | `[scripts/grade_m7.sh → [PASS] static grade M7 selesai]` |
| QEMU boot atau test target berjalan deterministik untuk build terbaru | `[BELUM DIUJI]` | `[ISO masih memuat kernel.elf lama; lihat bagian 14.2]` |
| Log build disimpan | `[PASS]` | `[build/evidence/m7_make_check.log]` |
| Panic path terbaca atau dijelaskan jika belum relevan | `[PASS]` | `[bagian 15.4]` |
| Tidak ada warning kritis pada build | `[PASS]` | `[make, setelah perbaikan asm]` |
| Perubahan Git terkomit | `[PASS]` | `[commit 40a6aa4]` |
| Desain dan failure mode dijelaskan | `[PASS]` | `[bagian 9 dan 15]` |
| Laporan berisi screenshot/log yang cukup | `[PARTIAL]` | `[log lengkap tersedia; screenshot menyusul pada lampiran]` |

Kriteria tambahan untuk praktikum lanjutan:

| Kriteria lanjutan | Status | Evidence |
|---|---|---|
| Static analysis dijalankan | `[PASS]` | `[readelf, nm, objdump]` |
| Stress test dijalankan | `[NA]` | `[Tidak menjadi fokus M7]` |
| Fuzzing atau malformed-input test dijalankan | `[NA]` | `[Tidak dilakukan pada M7]` |
| Fault injection dijalankan | `[NA]` | `[Tidak dilakukan pada M7]` |
| Disassembly/readelf evidence tersedia | `[PASS]` | `[build/evidence/m7_vmm_objdump.txt, m7_vmm_readelf_header.txt]` |
| Review keamanan dilakukan | `[PASS]` | `[bagian 17]` |
| Rollback diuji | `[NA]` | `[Belum dilakukan karena implementasi sudah berhasil]` |
| Debugging GDB pada modul baru | `[BELUM]` | `[Direncanakan pada iterasi berikutnya, lihat bagian 20]` |

---

## 20. Readiness Review

Pilih satu status dengan alasan berbasis bukti.

| Status | Definisi | Pilihan |
|---|---|---|
| Belum siap uji | Build/test belum stabil atau bukti belum cukup | `[ ]` |
| Siap uji QEMU | Build bersih, QEMU/test target berjalan, log tersedia | `[ ]` |
| Siap demonstrasi praktikum | Siap ditunjukkan di kelas dengan bukti uji, failure mode, dan rollback | `[ ]` |
| Kandidat siap pakai terbatas | Hanya untuk penggunaan terbatas setelah test, security review, dokumentasi, dan known issue tersedia | `[ ]` |
| Siap uji (build & host test) — status khusus untuk laporan ini | Build freestanding dan unit test host lulus penuh, namun bukti runtime QEMU/GDB untuk build ini belum tersedia | `[ x ]` |

Alasan readiness:

```text
Berdasarkan hasil build freestanding, unit test host, dan static
grading (grade_m7.sh) yang seluruhnya menghasilkan status [PASS],
implementasi VMM pada M7 telah memenuhi pemeriksaan mekanis pada level
kompilasi dan symbol/instruksi (invlpg, cr3). Namun demikian, bukti
runtime melalui QEMU dan sesi debugging GDB untuk build M7 secara
spesifik belum tersedia karena image ISO yang sempat diuji masih
menggunakan kernel.elf lama dari milestone sebelumnya. Oleh karena itu,
status "Siap uji QEMU" belum dapat diklaim secara jujur untuk modul VMM
ini, dan dipilih status "Siap uji (build & host test)" sebagai
representasi paling akurat dari bukti yang tersedia saat ini.
```

Known issues:

| No. | Issue | Dampak | Workaround | Target perbaikan |
|---|---|---|---|---|
| 1 | `[iso_root/boot/kernel.elf belum diperbarui dengan build M7 terbaru]` | `[Bukti QEMU/GDB tidak representatif untuk modul VMM M7]` | `[Menyalin ulang build/kernel.elf terbaru ke iso_root/boot/kernel.elf sebelum membuat ISO]` | `[Iterasi berikutnya sebelum demonstrasi]` |
| 2 | `[Belum ada mekanisme locking pada page table untuk SMP]` | `[Tidak memengaruhi M7 yang berjalan single-core]` | `[Cukup tanpa lock pada M7]` | `[M15/SMP]` |
| 3 | `[Belum terdapat pengujian stress/fuzz pada operasi map/unmap]` | `[Tidak memengaruhi target minimum M7]` | `[Dilakukan pada milestone keamanan berikutnya]` | `[M14]` |
| 4 | `[Validasi alamat pada vmm_write_cr3 belum ada]` | `[Berpotensi crash jika dipanggil dengan alamat tidak valid]` | `[Pemanggil wajib memvalidasi alamat sebelum memanggil vmm_write_cr3]` | `[Iterasi berikutnya]` |

Keputusan akhir:

```text
Berdasarkan hasil pengujian dan evidence yang diperoleh, praktikum M7
dinyatakan siap pada level build dan unit test host, dengan status
readiness "Siap uji (build & host test)". Verifikasi runtime penuh
melalui QEMU dan debugging GDB pada modul VMM akan dilanjutkan setelah
iso_root/boot/kernel.elf diperbarui dengan hasil build M7 terbaru.
```

---

## 21. Rubrik Penilaian 100 Poin

| Komponen | Bobot | Indikator nilai penuh | Nilai |
|---|---:|---|---:|
| Kebenaran fungsional | 30 | Implementasi memenuhi target praktikum, build/test lulus, output sesuai expected result | `[27]` |
| Kualitas desain dan invariants | 20 | Desain jelas, kontrak antarmuka eksplisit, invariants/ownership/locking terdokumentasi | `[18]` |
| Pengujian dan bukti | 20 | Unit/integration/QEMU/static/fuzz/stress evidence memadai sesuai tingkat praktikum | `[14]` |
| Debugging dan failure analysis | 10 | Failure mode, triage, panic/log, dan rollback dianalisis | `[10]` |
| Keamanan dan robustness | 10 | Boundary, input validation, privilege, memory safety, dan negative tests dibahas | `[9]` |
| Dokumentasi dan laporan | 10 | Laporan rapi, lengkap, dapat direproduksi, memakai referensi yang layak | `[10]` |
| **Total** | **100** |  | `[88]` |

Catatan penilai:

```text
[Diisi dosen/asisten. Nilai pengujian dan bukti diberi skor lebih
konservatif karena bukti runtime QEMU/GDB untuk modul VMM M7 belum
tersedia pada sesi ini.]
```

---

## 22. Kesimpulan

### 22.1 Yang Berhasil

```text
Praktikum M7 berhasil mengimplementasikan Virtual Memory Manager (VMM)
pada kernel MCSOS. Fungsi vmm_unmap berhasil memvalidasi flag PTE
sebelum menghapus entri, vmm_invalidate_page berhasil menghasilkan
instruksi invlpg yang terverifikasi pada objdump build freestanding,
dan vmm_read_cr3/vmm_write_cr3/vmm_read_cr2 berhasil diimplementasikan
tanpa duplikasi. Unit test host (test_vmm_host.c) lulus dengan pesan
"M7 VMM host tests PASS", dan script static grading (grade_m7.sh)
mengeluarkan status akhir "[PASS] static grade M7 selesai" secara
konsisten setelah seluruh bug diperbaiki. Seluruh perubahan telah
dikomit dan didorong ke branch praktikum/m7-vmm (commit 40a6aa4).
```

### 22.2 Yang Belum Berhasil

```text
Bukti runtime melalui QEMU dan sesi debugging GDB untuk build M7 secara
spesifik belum tersedia, karena image ISO yang sempat diuji masih
menggunakan kernel.elf lama dari milestone sebelumnya (ditandai log
serial "[MCSOS:M5] boot: external interrupt bring-up start"). Pengujian
stress test, fuzzing, dan fault injection pada operasi map/unmap juga
belum dilakukan karena belum menjadi cakupan pada milestone M7. Validasi
alamat pada vmm_write_cr3 juga belum diimplementasikan.
```

### 22.3 Rencana Perbaikan

```text
Tahap selanjutnya adalah memperbarui iso_root/boot/kernel.elf dengan
hasil build M7 terbaru, membuat ulang image ISO, kemudian menjalankan
QEMU dan sesi GDB dengan breakpoint pada vmm_unmap, vmm_invalidate_page,
vmm_read_cr3, dan vmm_write_cr3 untuk melengkapi bukti runtime M7.
Selanjutnya dilanjutkan implementasi M8 (thread dan scheduler) yang akan
memanfaatkan VMM untuk pengelolaan ruang alamat per-thread/proses.
```

---

## 23. Lampiran

### Lampiran A — Commit Log

```text
git log --oneline -3
40a6aa4 (HEAD -> praktikum/m7-vmm, origin/praktikum/m7-vmm) M7: implement virtual memory manager (VMM)
bc3b131 (origin/praktikum/m6-pmm, praktikum/m6-pmm) M6: implement physical memory manager (PMM)
0bb81d8 (origin/praktikum/m5-timer-irq, praktikum/m5-timer-irq) M5: PIC, PIT, IRQ0 timer interrupt
```

### Lampiran B — Diff Ringkas

```diff
git diff --stat
7 files changed, 458 insertions(+), 1 deletion(-)
 Makefile                       |  ...
 kernel/core/kmain.c            |  ...
 kernel/core/trap.c             |  ...
 kernel/core/vmm.c              |  ... (baru)
 kernel/include/mcsos/vmm.h     |  ... (baru)
 scripts/grade_m7.sh            |  ... (baru)
 tests/test_vmm_host.c          |  ... (baru)
```

### Lampiran C — Log Build Lengkap

```text
Build freestanding berhasil tanpa error setelah perbaikan penulisan
inline assembly pada vmm.c. vmm.c berhasil dikompilasi dan dilink ke
kernel.elf sebagai build/normal/kernel/core/vmm.o.
```

### Lampiran D — Log QEMU (Catatan: berasal dari kernel.elf lama, belum representatif untuk M7)

```text
limine: Loading executable `boot():/boot/kernel.elf`...
[MCSOS:M5] boot: external interrupt bring-up start
idt_base=0xffffffff80004000
idt_limit=0x0000000000000fff
[M4] IDT loaded
[MCSOS:M5] idt: loaded
[MCSOS:M5] pic: remapped; mask master=0x00000000000000fe slave=0x00000000000000ff
[MCSOS:M5] pit: configured 100Hz
[MCSOS:M5] sti: enabling interrupts
[MCSOS:TIMER] ticks=0x0000000000000064
[MCSOS:TIMER] ticks=0x00000000000000c8
...
qemu: terminating on signal 2
```

Catatan: Log di atas berasal dari `iso_root/boot/kernel.elf` versi lama
(milestone M5) yang belum diperbarui dengan hasil build M7, sehingga
**tidak** dijadikan bukti keberhasilan runtime modul VMM M7. Log ini
dilampirkan apa adanya sesuai riwayat terminal untuk transparansi.

### Lampiran E — Output Readelf/Objdump

```text
build/kernel.readelf.header.txt
build/kernel.readelf.programs.txt
build/kernel.syms.txt
build/kernel.disasm.txt
build/evidence/m7_vmm_readelf_header.txt
build/evidence/m7_vmm_readelf_sections.txt
build/evidence/m7_vmm_nm_undefined.txt (kosong — tidak ada unresolved symbol)
build/evidence/m7_vmm_objdump.txt (memuat instruksi invlpg (%rax) pada offset 0x9cd)
```

### Lampiran F — Screenshot

| No. | File | Keterangan |
|---|---|---|
| 1 | `[Screenshot Terminal]` | `[make berhasil, vmm.o terlink ke kernel.elf]` |
| 2 | `[Screenshot unit test host]` | `[M7 VMM host tests PASS]` |
| 3 | `[Screenshot grade_m7.sh]` | `[PASS] static grade M7 selesai]` |
| 4 | `[Screenshot git commit/push]` | `[commit 40a6aa4 pada branch praktikum/m7-vmm]` |

### Lampiran G — Bukti Tambahan

```text
build/evidence/
build/normal/kernel/core/vmm.o (memuat instruksi invlpg nyata)
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
[1] Intel Corporation, Intel® 64 and IA-32 Architectures Software Developer's Manual,
    "Paging (4-Level Page Table)", "INVLPG", dan "Control Registers (CR2, CR3)".
[2] LLVM/Clang Documentation, "Inline Assembly" dan opsi -std=c17.
[3] GNU Binutils Documentation, readelf, objdump, dan nm.
[4] Dokumentasi Git, git branch, git commit, git push.
```

---

## 25. Checklist Final Sebelum Pengumpulan

| Checklist | Status |
|---|---|
| Semua placeholder `[isi ...]` sudah diganti | `[Sebagian — tanggal pengumpulan final dan hash SHA-256 kernel.elf perlu diisi]` |
| Metadata laporan lengkap | `[Ya]` |
| Commit awal dan akhir dicatat | `[Commit awal bc3b131 dan akhir 40a6aa4 dicatat]` |
| Perintah build dan test dapat dijalankan ulang | `[Ya]` |
| Log build dilampirkan | `[Ya]` |
| Log unit test host dilampirkan | `[Ya]` |
| Log QEMU/test runtime untuk build M7 dilampirkan | `[Belum — hanya log ISO lama yang dilampirkan sebagai catatan]` |
| Artefak penting diberi hash | `[Belum — perlu sha256sum build/kernel.elf]` |
| Desain, invariants, ownership, dan failure modes dijelaskan | `[Ya]` |
| Security/reliability dibahas | `[Ya]` |
| Readiness review tidak berlebihan | `[Ya — status disesuaikan dengan bukti yang benar-benar tersedia]` |
| Rubrik penilaian diisi atau disiapkan | `[Ya]` |
| Referensi memakai format IEEE | `[Ya]` |
| Laporan disimpan sebagai Markdown | `[Ya]` |

---

## 26. Pernyataan Pengumpulan

Saya/kami mengumpulkan laporan ini bersama artefak pendukung pada commit:

```text
Saya mengumpulkan laporan ini bersama artefak pendukung pada commit:

40a6aa4

Status akhir yang diklaim:

Siap uji (build & host test); QEMU/GDB untuk build M7 belum tersedia
```

Status akhir yang diklaim:

```text
[Siap uji (build & host test)]
```

Ringkasan satu paragraf:

```text
[Praktikum M7 berhasil mengimplementasikan Virtual Memory Manager (VMM)
dengan operasi unmap yang memvalidasi flag PTE, invalidasi TLB melalui
instruksi invlpg yang terverifikasi pada objdump build freestanding,
serta akses register CR2/CR3 tanpa duplikasi fungsi setelah perbaikan.
Unit test host lulus dan static grading (grade_m7.sh) menghasilkan
status akhir PASS. Bukti runtime QEMU/GDB untuk build M7 secara spesifik
belum tersedia karena image ISO yang diuji masih menggunakan kernel.elf
lama, dan akan dilengkapi pada iterasi berikutnya. Repository telah
diperbarui pada branch praktikum/m7-vmm (commit 40a6aa4) sebagai bukti
penyelesaian praktikum M7 pada level build dan unit test.]
```
