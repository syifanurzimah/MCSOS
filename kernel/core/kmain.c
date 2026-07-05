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

void kmain(void)
{

    cpu_cli();

    log_init();
    log_writeln("[MCSOS:M5] boot: external interrupt bring-up start");
    x86_64_idt_init();
    log_writeln("[MCSOS:M5] idt: loaded");

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


    if (mcsos_scheduler_init(&g_sched, &g_boot_thread)
        != MCSOS_SCHED_OK) {
        KERNEL_PANIC("M9 scheduler init failed", 1);
    }

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
