#include <mcsos/arch/cpu.h>
#include <mcsos/arch/idt.h>
#include <mcsos/arch/pic.h>
#include <mcsos/arch/pit.h>
#include <mcsos/kernel/log.h>
#include <mcsos/kernel/panic.h>
#include <mcsos/kmem.h>
#include <mcsos/pmm.h>
#include <mcsos/vmm.h>

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

#define M8_BOOT_HEAP_SIZE (64u * 1024u)

static unsigned char m8_boot_heap[M8_BOOT_HEAP_SIZE]
    __attribute__((aligned(4096)));

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

    log_writeln("[MCSOS:M5] sti: enabling interrupts");
    cpu_sti();

    for (;;) {
        cpu_hlt();
    }
}
