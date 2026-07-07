#include <mcsos/arch/cpu.h>
#include <mcsos/arch/idt.h>
#include <mcsos/arch/pic.h>
#include <mcsos/arch/pit.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>
#include <mcsos/kmem.h>
#include <mcsos/pmm.h>
#include <mcsos/vmm.h>
#include <mcsos/mcsos_thread.h>
#include <mcsos/user/m11_elf_loader.h>
#include "../../include/mcsos/syscall.h"
#include <mcs_sync_selftest.h>

extern void x86_64_syscall_int80_stub(void);

static __attribute__((unused))
uint64_t kernel_vmm_alloc(void *ctx)
{
    return pmm_alloc_frame((struct pmm_state *)ctx);
}

static __attribute__((unused))
void kernel_vmm_free(void *ctx, uint64_t frame_paddr)
{
    pmm_free_frame((struct pmm_state *)ctx, frame_paddr);
}

static __attribute__((unused))
void *kernel_phys_to_virt(void *ctx, uint64_t paddr)
{
    const uint64_t hhdm_offset = *(const uint64_t *)ctx;
    return (void *)(hhdm_offset + paddr);
}

static __attribute__((unused))
struct vmm_space kernel_space;

static mcsos_scheduler_t g_sched;
static mcsos_thread_t g_boot_thread;
static mcsos_thread_t g_thread_a;
static mcsos_thread_t g_thread_b;

static unsigned char g_stack_a[8192]
    __attribute__((aligned(16)));

static uint64_t k_get_ticks(void)
{
    return timer_ticks();
}

static void k_yield_current(void)
{
    (void)mcsos_sched_yield(&g_sched);
}

static void k_exit_current(int code)
{
    (void)code;
    log_writeln("[M10] thread_exit stub");
}

static int64_t k_write_serial(const char *buf, size_t len)
{
    if (buf == 0)
        return -1;

    for (size_t i = 0; i < len; ++i) {
        char tmp[2];
        tmp[0] = buf[i];
        tmp[1] = '\0';
        log_write(tmp);
    }

    return (int64_t)len;
}

static unsigned char g_stack_b[8192]
    __attribute__((aligned(16)));

#define M8_BOOT_HEAP_SIZE (64u * 1024u)

static unsigned char m8_boot_heap[M8_BOOT_HEAP_SIZE]
    __attribute__((aligned(4096)));

static mcsos_scheduler_t g_sched;
static mcsos_thread_t g_boot_thread;
static mcsos_thread_t g_thread_a;
static mcsos_thread_t g_thread_b;

static unsigned char g_stack_a[8192]
    __attribute__((aligned(16)));

static unsigned char g_stack_b[8192]
    __attribute__((aligned(16)));

static void m8_heap_bootstrap(void)
{
    int rc = kmem_init(m8_boot_heap, sizeof(m8_boot_heap));
    if (rc != 0) {
        KERNEL_PANIC("M8 kmem_init failed", rc);
    }

    void *probe = kmem_alloc(128);
    if (probe == 0) {
        KERNEL_PANIC("M8 kmem_alloc probe failed", 1);
    }

    if (kmem_free_checked(probe) != 0) {
        KERNEL_PANIC("M8 kmem_free_checked probe failed", 2);
    }

    log_writeln("[M8] kernel heap initialized");
}

static void demo_thread_a(void *arg)
{
    (void)arg;

    for (;;) {
        log_writeln("[M9] thread A tick");
        mcsos_sched_yield(&g_sched);
    }
}

static void demo_thread_b(void *arg)
{
    (void)arg;

    for (;;) {
        log_writeln("[M9] thread B tick");
        mcsos_sched_yield(&g_sched);
    }
}
static void m11_loader_smoke(void)
{
    log_writeln("[M11] smoke entered");
    unsigned char image[4096 * 3] = {0};

    struct m11_elf64_ehdr *eh =
        (struct m11_elf64_ehdr *)(void *)image;

    eh->e_ident[0] = M11_ELFMAG0;
    eh->e_ident[1] = M11_ELFMAG1;
    eh->e_ident[2] = M11_ELFMAG2;
    eh->e_ident[3] = M11_ELFMAG3;
    eh->e_ident[4] = M11_ELFCLASS64;
    eh->e_ident[5] = M11_ELFDATA2LSB;
    eh->e_ident[6] = M11_EV_CURRENT;

    eh->e_type = M11_ET_EXEC;
    eh->e_machine = M11_EM_X86_64;
    eh->e_version = M11_EV_CURRENT;

    eh->e_entry = 0x401000;
    eh->e_phoff = sizeof(struct m11_elf64_ehdr);
    eh->e_ehsize = sizeof(struct m11_elf64_ehdr);
    eh->e_phentsize = sizeof(struct m11_elf64_phdr);
    eh->e_phnum = 1;

    struct m11_elf64_phdr *ph =
        (struct m11_elf64_phdr *)(void *)(image + eh->e_phoff);

    ph->p_type = M11_PT_LOAD;
    ph->p_flags = M11_PF_R | M11_PF_X;
    ph->p_offset = 0x1000;
    ph->p_vaddr = 0x401000;
    ph->p_filesz = 16;
    ph->p_memsz = 4096;
    ph->p_align = M11_PAGE_SIZE;


    struct m11_process_image_plan plan;

    struct m11_user_region region = {
        .base = 0x400000,
        .limit = 0x800000000000
    };


    int rc = m11_elf64_plan_load(
        image,
        sizeof(image),
        region,
        &plan
    );


    if (rc != M11_OK) {
        log_write("[M11] loader failed: ");
        log_writeln(m11_error_name(rc));
        return;
    }


    log_writeln("[M11] elf: plan ok");
    log_write("[M11] entry=");
    log_hex64(plan.entry);
    log_write(" segments=");
    log_hex64(plan.segment_count);
    log_writeln("");

    log_writeln("[M11] user image plan ready");
}
static void m10_syscall_smoke_direct(void)
{
    int64_t r = mcsos_syscall_dispatch(
        MCSOS_SYS_PING,
        0, 0, 0, 0, 0, 0);

    if (r != 0x2605020A) {
        KERNEL_PANIC("M10 syscall ping failed", r);
    }

    log_writeln("[M10] syscall ping ok");

    r = mcsos_syscall_dispatch(
        MCSOS_SYS_GET_TICKS,
        0, 0, 0, 0, 0, 0);

    if (r < 0) {
        KERNEL_PANIC("M10 syscall get_ticks failed", r);
    }

    log_writeln("[M10] syscall get_ticks ok");
    log_writeln("[M10] syscall smoke done");
}

void kmain(void)
{

    cpu_cli();
    log_init();
    log_writeln("[MCSOS:M5] boot: external interrupt bring-up start");
    x86_64_idt_init();
    log_writeln("[MCSOS:M5] idt: loaded");

#if 0
x86_64_idt_set_gate(
    0x80,
    (uint64_t)x86_64_syscall_int80_stub,
    X86_64_IDT_GATE_INTERRUPT
);

log_writeln("[M10] syscall vector 0x80 installed");
#endif

    pic_remap(0x20, 0x28);
    pic_mask_all();
    pic_unmask_irq(0);

    log_write("[MCSOS:M5] pic: remapped; mask master=");
    log_hex64(pic_read_master_mask());
    log_write(" slave=");
    log_hex64(pic_read_slave_mask());
    log_writeln("");

    pit_configure_hz(100);
    log_writeln("[MCSOS:M5] pit: configured 100Hz");

    m8_heap_bootstrap();
m12_sync_selftest();

    if (mcsos_scheduler_init(&g_sched, &g_boot_thread)
        != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 scheduler init failed", 1);
    }

mcsos_syscall_ops_t ops = {
    .get_ticks = k_get_ticks,
    .yield_current = k_yield_current,
    .exit_current = k_exit_current,
    .write_serial = k_write_serial,
};

mcsos_syscall_init(&ops);

#define MCSOS_USER_BASE  0x0000000000400000ULL
#define MCSOS_USER_LIMIT 0x0000800000000000ULL

mcsos_syscall_set_user_region((mcsos_user_region_t){
    .base = MCSOS_USER_BASE,
    .limit = MCSOS_USER_LIMIT,
});

log_writeln("[M10] user region initialized");
log_writeln("[M10] syscall subsystem initialized");


m10_syscall_smoke_direct();
log_writeln("[M11] before call");
m11_loader_smoke();
log_writeln("[M11] after call");
    mcsos_thread_prepare(
        &g_thread_a,
        "demo-a",
        demo_thread_a,
        0,
        g_stack_a,
        sizeof(g_stack_a),
        g_sched.next_id++);

    mcsos_thread_prepare(
        &g_thread_b,
        "demo-b",
        demo_thread_b,
        0,
        g_stack_b,
        sizeof(g_stack_b),
        g_sched.next_id++);

    mcsos_sched_enqueue(&g_sched, &g_thread_a);
    mcsos_sched_enqueue(&g_sched, &g_thread_b);

    log_writeln("[M9] scheduler initialized");

    log_writeln("[MCSOS:M5] sti: enabling interrupts");
    cpu_sti();

    for (;;) {
        cpu_hlt();
    }
}
