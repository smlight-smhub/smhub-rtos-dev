/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Main entry task and application dispatcher.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "cv181x_top_reg.h"
#include "uart.h"
#include <metal/sys.h>
#include <openamp/open_amp.h>
#include "rpmsg_app.h"
// External Application entry points
__attribute__((weak)) void app_setup(void) {}
__attribute__((weak)) void app_loop(void) { vTaskDelay(1000); }

extern const char ESPHOME_FIRMWARE_VERSION[];

void user_app_task(void *pvParameters) {
    rpmsg_app_init();
    
    uart_puts("Waiting for MAC address from Linux broker...\r\n");
    int poll_count = 0;
    while (!rpmsg_mac_received()) {
        rpmsg_process_queue();
        vTaskDelay(pdMS_TO_TICKS(10));
        
        poll_count++;
        if (poll_count >= 100) { // Every ~1 second
            poll_count = 0;
            // Explicitly request the MAC address in case Linux broker is already running
            rpmsg_request_mac();
        }
    }
    
    app_setup();
    
    while (1) {
        app_loop();

        // Check if the ISR flagged new data, and process it safely outside the interrupt context
        rpmsg_process_queue();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vPortSetupTimerInterrupt(void) {
    // Timer interrupt configured (C906 MTIME at 25MHz)
    
    // Set first tick to current time + 25000 (1ms at 25MHz)
    uint64_t mtime;
    __asm__ volatile("rdtime %0" : "=r"(mtime));
    // SG2000 heterogeneous C906 cores each have their own private CLINT.
    // They are Hart 0 within their local CLINT, so mtimecmp is at 0x74004000.
    // CRITICAL: The CLINT peripheral only accepts 32-bit accesses!
    // A 64-bit write (sd) will be silently dropped by the AXI bus!
    volatile uint32_t *mtimecmp_lo = (volatile uint32_t *)0x74004000;
    volatile uint32_t *mtimecmp_hi = (volatile uint32_t *)0x74004004;

    uint64_t next_match = mtime + 25000;

    // Write -1 to lo first to prevent spurious interrupts during the update
    *mtimecmp_lo = 0xFFFFFFFF;
    *mtimecmp_hi = (uint32_t)(next_match >> 32);
    *mtimecmp_lo = (uint32_t)(next_match);

    // Enable machine timer interrupt (MTIE, bit 7) AND external interrupt (MEIE, bit 11)
    // 0x80 (MTIE) | 0x800 (MEIE) = 0x880
    __asm__ volatile ("csrs mie, %0" :: "r"(0x880));
}

void freertos_risc_v_application_interrupt_handler(void) {
    uint64_t mcause;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause));

    if (mcause == 0x8000000000000007ULL) { // Machine Timer Interrupt
        uint64_t mtime;
        __asm__ volatile("rdtime %0" : "=r"(mtime));
        volatile uint32_t *mtimecmp_lo = (volatile uint32_t *)0x74004000;
        volatile uint32_t *mtimecmp_hi = (volatile uint32_t *)0x74004004;

        uint64_t next_match = mtime + 25000; // Schedule next 1ms tick
        *mtimecmp_lo = 0xFFFFFFFF;
        *mtimecmp_hi = (uint32_t)(next_match >> 32);
        *mtimecmp_lo = (uint32_t)(next_match);

        extern BaseType_t xTaskIncrementTick(void);
        extern void vTaskSwitchContext(void);
        if (xTaskIncrementTick() != pdFALSE) {
            vTaskSwitchContext();
        }
    } else if (mcause == 0x800000000000000bULL) { // Machine External Interrupt (PLIC)
        extern void do_irq(void);
        do_irq();
    } else {
        uart_puts("\r\n[!] Unhandled Interrupt: MCAUSE=");
        extern void uart_put_hex(uint64_t val); // From trap_c.c
        uart_put_hex(mcause);

        // Disable MEIE to prevent an interrupt loop for unknown external interrupts
        __asm__ volatile ("csrc mie, %0" :: "r"(0x800)); // Clear MEIE (bit 11)
    }
}

void freertos_risc_v_application_exception_handler(void) {
    uint64_t mcause, mepc, mtval;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause));
    __asm__ volatile ("csrr %0, mepc" : "=r"(mepc));
    __asm__ volatile ("csrr %0, mtval" : "=r"(mtval));

    uart_puts("\r\n================================================\r\n");
    uart_puts("   FATAL HARDWARE EXCEPTION (FreeRTOS TRAP)   \r\n");
    uart_puts("================================================\r\n");
    extern void uart_put_hex(uint64_t val);
    uart_puts("MCAUSE: "); uart_put_hex(mcause); uart_puts("\r\n");
    uart_puts("MEPC:   "); uart_put_hex(mepc); uart_puts("\r\n");
    uart_puts("MTVAL:  "); uart_put_hex(mtval); uart_puts("\r\n");
    while(1);
}

int main(void) {
    uart_init();

    uart_puts("\r\n\033[1;34m╔══════════════════════════════════════════════╗\033[0m\r\n");
    uart_puts("\033[1;34m║\033[0m          \033[1;37mSMLIGHT SMHUB  C906L RTOS\033[0m           \033[1;34m║\033[0m\r\n");
    uart_puts("\033[1;34m╚══════════════════════════════════════════════╝\033[0m\r\n\r\n");

    uart_puts("FW Version: ");
    const char *ver_start = ESPHOME_FIRMWARE_VERSION;
    while (*ver_start && *ver_start != ':') {
        ver_start++;
    }
    if (*ver_start == ':') {
        ver_start++; // Skip ':'
        while (*ver_start && *ver_start != '=') {
            char temp_char[2] = {*ver_start, '\0'};
            uart_puts(temp_char);
            ver_start++;
        }
    }
    uart_puts("\r\n\r\n");

    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    metal_init(&metal_params);

    // Create the User Application thread
    xTaskCreate(user_app_task, "UserApp", 16384, NULL, tskIDLE_PRIORITY + 1, NULL);

    // Start the FreeRTOS scheduler
    extern void freertos_risc_v_trap_handler(void);
    __asm__ volatile("csrw mtvec, %0" :: "r"(freertos_risc_v_trap_handler));
    vTaskStartScheduler();

    // Should never reach here
    while (1) {
    }
    return 0;
}
