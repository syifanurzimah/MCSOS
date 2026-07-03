#ifndef MCSOS_ARCH_IO_H
#define MCSOS_ARCH_IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
    return value;
}

static inline void io_wait(void) {
    outb(0x80u, 0u);
}

static inline void cli(void) {
    __asm__ volatile ("cli" : : : "memory");
}

static inline void sti(void) {
    __asm__ volatile ("sti" : : : "memory");
}

static inline void hlt(void) {
    __asm__ volatile ("hlt");
}

#endif
