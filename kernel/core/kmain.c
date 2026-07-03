#include <mcsos/arch/cpu.h>
#include <mcsos/arch/idt.h>
#include <mcsos/arch/pic.h>
#include <mcsos/arch/pit.h>
#include <mcsos/kernel/log.h>

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

    log_writeln("[MCSOS:M5] sti: enabling interrupts");
    cpu_sti();

    for (;;) {
        cpu_hlt();
    }
}

