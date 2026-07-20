/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Standard memory-mapped I/O primitives for hardware access.
 */
#ifndef __ASM_IO_H
#define __ASM_IO_H

#define readb(c)		({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)		({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)		({ u32 __v = readl_relaxed(c); __iormb(); __v; })
#define readq(c)		({ u64 __v = readq_relaxed(c); __iormb(); __v; })

#define writeb(v, c)		({ __iowmb(); writeb_relaxed((v), (c)); })
#define writew(v, c)		({ __iowmb(); writew_relaxed((v), (c)); })
#define writel(v, c)		({ __iowmb(); writel_relaxed((v), (c)); })
#define writeq(v, c)		({ __iowmb(); writeq_relaxed((v), (c)); })

#define ioread8 readb
#define ioread16 readw
#define ioread32 readl
#define ioread64 readq

#define iowrite8 writeb
#define iowrite16 writew
#define iowrite32 writel
#define iowrite64 writeq

#endif
