/*
 * FreeRTOS V202111.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://www.github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */

#include "riscv-virt.h"
.org 0
	.section .vectors, "ax"
	.globl _start
	.type _start,@function
_start:
	.cfi_startproc
	.cfi_undefined ra


	li x10, 0
	li x11, 0
	li x12, 0
	li x13, 0
	li x14, 0
	li x15, 0
	li x16, 0
	li x17, 0
	li x18, 0
	li x19, 0
	li x20, 0
	li x21, 0
	li x22, 0
	li x23, 0
	li x24, 0
	li x25, 0
	li x26, 0
	li x27, 0
	li x28, 0
	li x29, 0
	// Ensure all interrupts are explicitly masked!
	// The remoteproc driver triggers a true hardware reset via the reset pin,
	// which means the core natively boots in M-Mode.
	csrw mie, zero

	// Setup early bare-metal trap vector so we can see crash dumps before FreeRTOS starts!
	la t0, trap_vector
	csrw mtvec, t0
#ifndef RISCV_QEMU
	// invalidate all memory for BTB,BHT,DCACHE,ICACHE
	li x3, 0x30013
	csrs 0x7C2, x3
	// enable ICACHE,DCACHE,BHT,BTB,RAS,WA
	li x3, 0x7f
	csrs 0x7C1, x3
	// enable data_cache_prefetch, amr
	li x3, 0x610c
	csrs 0x7C5, x3 #mhint
#endif
	# enable fp
	li x3, 0x1 << 13
	csrs mstatus, x3

	// Primary hart
	la sp, _stack_top

	// Load data section
	la a0, _data_lma
	la a1, _data
	la a2, _edata
	bgeu a1, a2, 2f
1:
	ld t0, (a0)
	sd t0, (a1)
	addi a0, a0, 8
	addi a1, a1, 8
	bltu a1, a2, 1b
2:



	// Clear bss section
	la a0, _bss
	la a1, _ebss
	bgeu a0, a1, 2f
1:
	// reduce branch time, be sure about bss alignment in linker script
	sd zero, 0x00 (a0)
	sd zero, 0x08 (a0)
	sd zero, 0x10 (a0)
	sd zero, 0x18 (a0)
	sd zero, 0x20 (a0)
	sd zero, 0x28 (a0)
	sd zero, 0x30 (a0)
	sd zero, 0x38 (a0)
	addi a0, a0, 64
	bltu a0, a1, 1b
2:

	/* Call C++ global constructors */
	la s0, __init_array_start
	la s1, __init_array_end
3:
	bgeu s0, s1, 4f
	ld a2, 0(s0)
	jalr a2
	addi s0, s0, 8
	j 3b
4:

	// argc, argv, envp is 0
	li  a0, 0
	li  a1, 0
	li  a2, 0
	jal main
1:
	wfi
	j 1b



    .align 6
trap_vector:
  j trap_entry
  j die

trap_entry:
    addi sp, sp, -35 * 8
    sd x1, 1 * 8(sp)
    sd x2, 2 * 8(sp)
    sd x3, 3 * 8(sp)
    sd x4, 4 * 8(sp)
    sd x5, 5 * 8(sp)
    sd x6, 6 * 8(sp)
    sd x7, 7 * 8(sp)
    sd x8, 8 * 8(sp)
    sd x9, 9 * 8(sp)
    sd x10, 10 * 8(sp)
    sd x11, 11 * 8(sp)
    sd x12, 12 * 8(sp)
    sd x13, 13 * 8(sp)
    sd x14, 14 * 8(sp)
    sd x15, 15 * 8(sp)
    sd x16, 16 * 8(sp)
    sd x17, 17 * 8(sp)
    sd x18, 18 * 8(sp)
    sd x19, 19 * 8(sp)
    sd x20, 20 * 8(sp)
    sd x21, 21 * 8(sp)
    sd x22, 22 * 8(sp)
    sd x23, 23 * 8(sp)
    sd x24, 24 * 8(sp)
    sd x25, 25 * 8(sp)
    sd x26, 26 * 8(sp)
    sd x27, 27 * 8(sp)
    sd x28, 28 * 8(sp)
    sd x29, 29 * 8(sp)
    sd x30, 30 * 8(sp)
    sd x31, 31 * 8(sp)
    csrr a1, mepc
    sd a1,   0 * 8(sp)
    csrr t0, mstatus
    sd t0,  32 * 8(sp)
    csrr t1, mtval
    sd t1,  33 * 8(sp)
    csrr a0, mcause
    sd a0,  34 * 8(sp)
    mv a0, sp
    jal trap_c
die:
    j die

	.cfi_endproc
