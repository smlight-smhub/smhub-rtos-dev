/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Newlib C library to FreeRTOS memory mapping wrappers.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <semphr.h>
#include "uart.h"
#include "smhub_ipc.h"
#include <stdio.h>

// Map standard library memory functions to FreeRTOS
void *malloc(size_t size) {
    return pvPortMalloc(size);
}
void free(void *ptr) {
    vPortFree(ptr);
}
void *calloc(size_t nmemb, size_t size) {
    void *ptr = pvPortMalloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
void *realloc(void *ptr, size_t size) {
    if (!ptr) return pvPortMalloc(size);
    if (size == 0) { vPortFree(ptr); return NULL; }
    return NULL;
}

// Map newlib reentrant functions
void *_malloc_r(struct _reent *r, size_t size) {
    return pvPortMalloc(size);
}
void _free_r(struct _reent *r, void *ptr) {
    vPortFree(ptr);
}
void *_calloc_r(struct _reent *r, size_t nmemb, size_t size) {
    void *ptr = pvPortMalloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
void *_realloc_r(struct _reent *r, void *ptr, size_t size) {
    if (!ptr) return pvPortMalloc(size);
    if (size == 0) { vPortFree(ptr); return NULL; }
    return NULL;
}

void *_sbrk_r(struct _reent *r, ptrdiff_t incr) {
    uart_puts("ERROR: _sbrk_r called!\r\n");
    return (void *)-1;
}

int _isatty_r(struct _reent *r, int fd) {
    return 1;
}
int _fstat_r(struct _reent *r, int fd, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}
int _close_r(struct _reent *r, int fd) {
    return 0;
}
_off_t _lseek_r(struct _reent *r, int fd, _off_t ptr, int dir) {
    return 0;
}
_ssize_t _read_r(struct _reent *r, int fd, void *ptr, size_t len) {
    return 0;
}

// Stubs for unsupported system calls
int _getpid(void) {
    return 1;
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

static SemaphoreHandle_t g_entropy_sem = NULL;
static uint8_t *g_entropy_buf = NULL;
static size_t g_entropy_buflen = 0;
static size_t g_entropy_received = 0;

void esphome_rpmsg_entropy_response_pb(const smhub_hal_rpc_GetEntropyResp *resp) {
    if (g_entropy_buf && resp->random_data.size > 0) {
        size_t to_copy = (resp->random_data.size < g_entropy_buflen) ? resp->random_data.size : g_entropy_buflen;
        memcpy(g_entropy_buf, resp->random_data.bytes, to_copy);
        g_entropy_received = to_copy;
        printf("[ENTROPY] Received RPC response: %d bytes (first byte: 0x%02X)\r\n", 
               (int)to_copy, g_entropy_buf[0]);
    }

    if (g_entropy_sem) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(g_entropy_sem, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

int _getentropy(void *buf, size_t buflen) {
    if (buflen > 256) {
        errno = EIO;
        return -1;
    }

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        if (!g_entropy_sem) {
            g_entropy_sem = xSemaphoreCreateBinary();
        }

        g_entropy_buf = (uint8_t *)buf;
        g_entropy_buflen = buflen;
        g_entropy_received = 0;

        smhub_hal_rpc_RpcCommand cmd = smhub_hal_rpc_RpcCommand_init_zero;
        cmd.type = smhub_hal_rpc_CommandType_GET_ENTROPY_REQ;
        cmd.entropy_req.requested_size = buflen;

        printf("[ENTROPY] Sending RPC request for %d bytes\r\n", (int)buflen);

        if (smhub_ipc_send_rpc(&cmd)) {
            // Block for response (wait max 1000ms)
            if (xSemaphoreTake(g_entropy_sem, pdMS_TO_TICKS(1000)) == pdTRUE) {
                if (g_entropy_received == buflen) {
                    return 0; // Success!
                } else {
                    printf("[ENTROPY] RPC response size mismatch. Expected %d, got %d\r\n", (int)buflen, (int)g_entropy_received);
                }
            } else {
                printf("[ENTROPY] RPC request timed out!\r\n");
            }
        }
    }

    printf("[ENTROPY] Falling back to pseudo-random generator\r\n");

    // Fallback pseudo-random
    uint8_t *p = (uint8_t *)buf;

    uint64_t mcycle;
    __asm__ volatile ("csrr %0, mcycle" : "=r" (mcycle));

    uint32_t state = (uint32_t)mcycle ^ (uint32_t)(mcycle >> 32) ^ 0x12345678;

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        state ^= xTaskGetTickCount();
    }

    for (size_t i = 0; i < buflen; i++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        p[i] = (uint8_t)(state & 0xFF);
    }
    return 0;
}

// Timekeeping
static uint64_t g_epoch_offset_seconds = 0;
static uint32_t g_epoch_offset_ticks = 0;

int settimeofday(const struct timeval *tv, const struct timezone *tz) {
    if (tv) {
        g_epoch_offset_seconds = tv->tv_sec;
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            g_epoch_offset_ticks = xTaskGetTickCount();
        } else {
            g_epoch_offset_ticks = 0;
        }
    }
    return 0;
}

int _gettimeofday(struct timeval *tv, void *tz) {
    if (tv) {
        if (g_epoch_offset_seconds == 0) {
            tv->tv_sec = 0;
            tv->tv_usec = 0;
        } else {
            uint32_t ticks_now = 0;
            if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
                ticks_now = xTaskGetTickCount();
            }
            uint32_t diff_ticks = ticks_now - g_epoch_offset_ticks;
            uint32_t diff_seconds = diff_ticks / configTICK_RATE_HZ;
            uint32_t diff_ms = (diff_ticks % configTICK_RATE_HZ) * (1000 / configTICK_RATE_HZ);
            tv->tv_sec = g_epoch_offset_seconds + diff_seconds;
            tv->tv_usec = diff_ms * 1000;
        }
    }
    return 0;
}

// Dummy __dso_handle for bare-metal C++ static destructors
void *__dso_handle __attribute__((__weak__)) = NULL;
