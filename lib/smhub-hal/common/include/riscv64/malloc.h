#ifndef __MALLOC_H___
#define __MALLOC_H__
#include <stddef.h>

/* here use freertos malloc & free function*/
extern void *pvPortMalloc(size_t xWantedSize);
extern void vPortFree(void *pv);

void *memset(void *dest, int value, unsigned long size);
void *memcpy(void *dest, const void *source, unsigned long size);

/* Standard memory wrappers are now safely handled by newlib _malloc_r in main.c */

/* align addr on a size boundary - adjust address up/down if needed */
#define _ALIGN_UP(addr, size)	(((addr)+((size)-1))&(~((typeof(addr))(size)-1)))
#define _ALIGN_DOWN(addr, size)	((addr)&(~((typeof(addr))(size)-1)))

/* align addr on a size boundary - adjust address up if needed */
#define _ALIGN(addr,size)     _ALIGN_UP(addr,size)

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	_ALIGN(addr, PAGE_SIZE)

#endif // end of __MALLOC_H__
