/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * libmetal system-specific port functions for CV181X/SG2000.
 */
#include <metal/sys.h>
#include <metal/irq.h>
#include "arch_helpers.h"

// Disable all interrupts and return previous state
unsigned int sys_irq_save_disable(void) {
    unsigned long mstatus;
    // Read and clear the MIE (Machine Interrupt Enable) bit (bit 3)
    __asm__ volatile("csrrci %0, mstatus, 8" : "=r"(mstatus));
    return (mstatus & 8) ? 1 : 0;
}

// Restore interrupt state
void sys_irq_restore_enable(unsigned int flags) {
    if (flags) {
        // Set the MIE bit
        __asm__ volatile("csrrs zero, mstatus, 8");
    }
}

extern void enable_irq(unsigned int irqn);
extern void disable_irq(unsigned int irqn);

// Enable a specific interrupt in the PLIC
void sys_irq_enable(unsigned int vector) {
    enable_irq(vector);
}

// Disable a specific interrupt in the PLIC
void sys_irq_disable(unsigned int vector) {
    disable_irq(vector);
}

// Finished
// Machine specific cache flush
void metal_machine_cache_flush(void *addr, unsigned int len) {
    if (addr && len > 0) {
        flush_dcache_range((uintptr_t)addr, len);
    }
}

// Machine specific cache invalidate
void metal_machine_cache_invalidate(void *addr, unsigned int len) {
    if (addr && len > 0) {
        inv_dcache_range((uintptr_t)addr, len);
    }
}

// Machine specific IO mapping
void metal_machine_io_mem_map(void *va, metal_phys_addr_t pa, size_t size, unsigned int flags) {
    // Bare-metal RISC-V uses 1:1 physical-to-virtual memory mapping.
    // No MMU page table modifications are necessary.
}
