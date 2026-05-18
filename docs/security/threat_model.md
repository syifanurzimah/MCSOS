# Threat Model Awal MCSOS 260502 — M0

## Assets

| Asset | Alasan dilindungi |
|------|------------------|
| Source code repository | Menentukan perilaku kernel dan tools |
| Toolchain | Compiler/linker salah bisa menghasilkan binary salah |
| Build scripts | Bisa menyisipkan target atau flag berbahaya |
| Documentation baseline | Menjadi acuan requirement & acceptance |
| Generated artifacts | Bukti penilaian (log, image, map, dll) |
| Future signing keys | Belum dibuat di M0 tapi harus direncanakan |

---

## Actors

| Actor | Capability |
|------|-----------|
| Mahasiswa | Mengubah repo dan menjalankan build |
| Anggota tim | Modify branch & dokumentasi |
| Dosen/asisten | Review & evaluasi |
| Dependency eksternal | Supply package/tool |
| Local process | Bisa merusak file jika permission buruk |

---

## Trust Boundaries

1. Windows host ↔ WSL environment  
2. Source code ↔ build output  
3. Package manager ↔ local toolchain  
4. Script ↔ shell user  
5. Future QEMU guest ↔ host system  

---

## Initial Threats & Mitigations

| Threat | Impact | Mitigation |
|--------|--------|------------|
| Repo di `/mnt/c` | build tidak reproducible | wajib di WSL filesystem |
| Compiler tanpa target eksplisit | ABI salah | pakai `--target=x86_64-unknown-none` |
| Script build disisipi command | supply chain attack | review manual + git tracking |
| Dependency berubah versi | build beda hasil | lock & catat versi toolchain |
| Artifact tidak diverifikasi | bukti tidak valid | wajib `readelf/objdump/checksum` |
