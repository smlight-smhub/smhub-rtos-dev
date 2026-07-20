#ifndef _SMHUB_ARBITRATION_H_
#define _SMHUB_ARBITRATION_H_

#include <stdint.h>
#include <stdbool.h>

#define SMHUB_ARBITRATION_MAGIC_ADDR 0x8FFDFFF8
#define SMHUB_ARBITRATION_MASK_ADDR  0x8FFDFFFC
#define SMHUB_ARBITRATION_MAGIC_WORD 0x534D4857

// Hardware bitmask definitions
#define SMHUB_HW_SPI0  (1 << 0)
#define SMHUB_HW_SPI1  (1 << 1)
#define SMHUB_HW_SPI3  (1 << 2)
#define SMHUB_HW_I2C2  (1 << 3)
#define SMHUB_HW_I2C4  (1 << 4)
#define SMHUB_HW_UART2 (1 << 5)
#define SMHUB_HW_UART3 (1 << 6)
#define SMHUB_HW_PWM0  (1 << 7)
#define SMHUB_HW_PWM1  (1 << 8)
#define SMHUB_HW_PWM2  (1 << 9)
#define SMHUB_HW_PWM3  (1 << 10)
#define SMHUB_HW_ADC1  (1 << 11)
#define SMHUB_HW_ADC2  (1 << 12)

static inline bool smhub_hardware_is_released(uint32_t hw_bit) {
    volatile uint32_t *magic = (volatile uint32_t *)SMHUB_ARBITRATION_MAGIC_ADDR;
    volatile uint32_t *mask  = (volatile uint32_t *)SMHUB_ARBITRATION_MASK_ADDR;
    
    if (*magic != SMHUB_ARBITRATION_MAGIC_WORD) {
        return false; // Magic mismatch, Linux owns everything
    }
    
    return (*mask & hw_bit) != 0;
}

#endif // _SMHUB_ARBITRATION_H_
