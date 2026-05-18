# System Requirements MCSOS 260502 — Baseline M0

## Scope
Dokumen ini menetapkan requirement awal untuk proyek MCSOS 260502.
Requirement pada M0 berfokus pada lingkungan, governance, dan evidence.
Requirement runtime kernel akan diperinci pada milestone berikutnya.

## Functional Requirements

### FR-001 Kernel Boot
Kernel harus berhasil dijalankan di QEMU tanpa crash pada 5 detik pertama.

### FR-002 Serial Output
Kernel harus dapat mengirim output ke serial console (QEMU stdio).

### FR-003 Halt Behavior
Kernel harus masuk ke state halt setelah proses boot selesai.

## Non-Functional Requirements

### NFR-001 Reproducibility
Build harus bisa dijalankan ulang dengan hasil yang sama di environment Linux WSL.

### NFR-002 Toolchain Consistency
Semua tool yang digunakan harus tercatat di dokumentasi versi toolchain.

### NFR-003 Stability
Tidak boleh terjadi kernel panic saat proses boot M0.

## Evidence Requirements

Setiap requirement wajib memiliki bukti:

- log output
- command execution
- screenshot QEMU (jika ada)
- hasil `readelf` atau `objdump`
- commit hash Git
