/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * FreeRTOS chip-specific registers and context extension hooks for C906L core.
 */
#ifndef FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H
#define FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H

#define portasmHAS_MTIME 0
#define portasmADDITIONAL_CONTEXT_SIZE 0

#ifdef __ASSEMBLER__
.macro portasmSAVE_ADDITIONAL_REGISTERS
    /* No additional registers to save */
.endm

.macro portasmRESTORE_ADDITIONAL_REGISTERS
    /* No additional registers to restore */
.endm
#endif

#endif /* FREERTOS_RISC_V_CHIP_SPECIFIC_EXTENSIONS_H */
