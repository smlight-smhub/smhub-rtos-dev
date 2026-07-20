/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * RPMsg / OpenAMP firmware setup and core logic.
 */
#include "rpmsg_app.h"
#include <openamp/open_amp.h>
#include <metal/io.h>
#include <metal/cache.h>
#include <metal/alloc.h>
#include "rsc_table.h"
#include <FreeRTOS.h>
#include <task.h>
#include "uart.h"



static struct rpmsg_virtio_device rvdev;
static struct rpmsg_virtio_shm_pool shpool;
static struct virtio_device vdev;
static struct virtqueue vqs[2];
static struct virtio_vring_info vrings[2];
static struct metal_io_region shm_io;
static struct metal_io_region vring_io[2];
static struct rpmsg_endpoint lept;
static struct rpmsg_endpoint rpc_ept;



volatile bool g_virtqueue_pending = false;

// Mailbox ISR
static int mailbox_irq_handler(int irq, void *priv) {
    // Clear the interrupt for CPU 2 (Channel 0)
    *((volatile uint32_t *)(uintptr_t)(0x01900030)) = 0x00000001; // cpu2_mbox_int_clr (write 1 to clear)

    // HW ACK: The Linux mailbox driver (cvi_mailbox.c) checks this bit to know if the channel is idle!
    // We strictly clear BIT(0) since DT '<&mailbox 0 2>' maps to Channel 0.
    *((volatile uint32_t *)(uintptr_t)(0x01900008)) &= ~0x00000001;

    // DEFER PROCESSING: Do not call virtqueue_notification() inside the ISR!
    g_virtqueue_pending = true;
    
    return 0;
}

static metal_phys_addr_t shm_phys = 0x8fc10000;
static metal_phys_addr_t vring_tx_phys = VRING_TX_ADDRESS;
static metal_phys_addr_t vring_rx_phys = VRING_RX_ADDRESS;

static unsigned char rsc_virtio_get_status(struct virtio_device *v) {
    metal_cache_invalidate(&resources.vdev.status, 1);
    return resources.vdev.status;
}

static void rsc_virtio_set_status(struct virtio_device *v, unsigned char status) {
    resources.vdev.status = status;
}

static uint32_t rsc_virtio_get_features(struct virtio_device *v) {
    return resources.vdev.dfeatures;
}

static void rsc_virtio_set_features(struct virtio_device *v, uint32_t features) {
    resources.vdev.gfeatures = features;
}

static void rsc_virtio_notify(struct virtqueue *vq) {
    
    // Vendor KICK sequence for CPU 1 (C906B Linux Host)
    // The Linux DT uses <&mailbox 1 1> for RX, meaning CPU 1, Channel 1.
    uint32_t target_cpu = 1;
    uint32_t channel = 1;
    uint32_t channel_mask = (1 << channel);
    
    // 1. Clear the interrupt for the target CPU (cpu_mbox_int_clr at 0x20 for CPU 1)
    *((volatile uint32_t *)(uintptr_t)(0x01900000 + 0x10 + (target_cpu * 0x10))) = channel_mask;
    
    // 2. Enable the mailbox channel for the target CPU (cpu_mbox_en at 0x04 for CPU 1)
    *((volatile uint32_t *)(uintptr_t)(0x01900000 + (target_cpu * 4))) |= channel_mask;
    
    *((volatile uint32_t *)(uintptr_t)0x01900060) = channel_mask;
}

static const struct virtio_dispatch dispatch = {
    .get_status = rsc_virtio_get_status,
    .set_status = rsc_virtio_set_status,
    .get_features = rsc_virtio_get_features,
    .set_features = rsc_virtio_set_features,
    .notify = rsc_virtio_notify,
};

extern void esphome_rpmsg_rx_cb(const uint8_t *data, size_t len);
extern void esphome_rpmsg_reset(void) __attribute__((weak));

uint8_t g_mac_address[6] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
bool g_mac_received = false;

bool rpmsg_mac_received(void) {
    return g_mac_received;
}

void sg2000_set_mac(const uint8_t* mac) {
    for(int i=0; i<6; i++) g_mac_address[i] = mac[i];
    g_mac_received = true;
    uart_puts("Injected MAC address received from Linux!\r\n");
}

static int endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv) {
    if (ept->dest_addr != RPMSG_ADDR_ANY && ept->dest_addr != src) {
        uart_puts("RTOS Endpoint Port changed! Resetting connection...\r\n");
        if (esphome_rpmsg_reset) {
            esphome_rpmsg_reset();
        }
    }

    if (ept->dest_addr != src) {
        ept->dest_addr = src;
        uart_puts("RTOS Endpoint Bound to Linux Address!\r\n");
    }

    if (len > 0) {
        esphome_rpmsg_rx_cb((const uint8_t *)data, len);
    }
    return RPMSG_SUCCESS;
}

extern void smhub_ipc_handle_rx(const uint8_t *payload, size_t len);

static int rpc_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv) {
    if (ept->dest_addr != src) {
        ept->dest_addr = src;
        uart_puts("RTOS RPC Endpoint Bound to Linux Address!\r\n");
    }

    if (len > 0) {
        smhub_ipc_handle_rx((const uint8_t *)data, len);
    }
    return RPMSG_SUCCESS;
}

void esphome_rpmsg_tx(const uint8_t *data, size_t len) {
    if (lept.dest_addr != RPMSG_ADDR_ANY) {
        size_t offset = 0;
        while (offset < len) {
            size_t chunk = len - offset;
            if (chunk > 496) chunk = 496;

            TickType_t start_tick = xTaskGetTickCount();
            while (rpmsg_send(&lept, data + offset, chunk) < 0) {
                if (lept.dest_addr == RPMSG_ADDR_ANY) return;
                if (xTaskGetTickCount() - start_tick > pdMS_TO_TICKS(100)) {
                    uart_puts("esphome_rpmsg_tx: send timeout, resetting dest_addr!\r\n");
                    lept.dest_addr = RPMSG_ADDR_ANY;
                    if (esphome_rpmsg_reset) {
                        esphome_rpmsg_reset();
                    }
                    return;
                }
                rpmsg_process_queue();
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            offset += chunk;
        }
    }
}

void rpc_rpmsg_tx(const uint8_t *data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        size_t chunk = len - offset;
        if (chunk > 496) chunk = 496;

        if (rpc_ept.dest_addr == RPMSG_ADDR_ANY) {
            rpmsg_sendto(&rpc_ept, (void*)(data + offset), chunk, 1025);
        } else {
            TickType_t start_tick = xTaskGetTickCount();
            while (rpmsg_send(&rpc_ept, data + offset, chunk) < 0) {
                if (rpc_ept.dest_addr == RPMSG_ADDR_ANY) break;
                if (xTaskGetTickCount() - start_tick > pdMS_TO_TICKS(100)) {
                    uart_puts("rpc_rpmsg_tx: send timeout, resetting dest_addr!\r\n");
                    rpc_ept.dest_addr = RPMSG_ADDR_ANY;
                    break;
                }
                rpmsg_process_queue();
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        }
        offset += chunk;
    }
}


extern uint8_t connection_established;
static void endpoint_unbind_cb(struct rpmsg_endpoint *ept) {
    uart_puts("RPMSG Unbound!\r\n");
    ept->dest_addr = RPMSG_ADDR_ANY;
    if (ept == &lept) {
        connection_established = 0;
    }
}

static void ns_bind_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest) {
    uart_puts("Name Service Bind: ");
    uart_puts((char*)name);
    uart_puts("\r\n");
    
    if (strcmp(name, "esphome-rpc") == 0) {
        if (lept.dest_addr == RPMSG_ADDR_ANY) {
            lept.dest_addr = dest;
        }
    } else if (strcmp(name, "smhub-rpc") == 0) {
        if (rpc_ept.dest_addr == RPMSG_ADDR_ANY) {
            rpc_ept.dest_addr = dest;
        }
    }
}

void rpmsg_app_init(void) {
    metal_io_init(&shm_io, (void *)0x8fc10000, &shm_phys, 0x100000, (unsigned int)-1, 0, NULL);
    metal_io_init(&vring_io[0], (void *)VRING_TX_ADDRESS, &vring_tx_phys, 0x8000, (unsigned int)-1, 0, NULL);
    metal_io_init(&vring_io[1], (void *)VRING_RX_ADDRESS, &vring_rx_phys, 0x8000, (unsigned int)-1, 0, NULL);
    
    // Linux DT <&mailbox 0 2> means Channel = 0, Target CPU = 2!
    // We STRICTLY enable ONLY Channel 0 (Bit 0 = 0x01) on CPU 2.
    *((volatile uint32_t *)(uintptr_t)(0x01900008)) = 0x00000001; // CPU 2 Enable ONLY Channel 0

    rpmsg_virtio_init_shm_pool(&shpool, (void *)0x8fc10000, 0x100000);

    vdev.role = RPMSG_REMOTE;
    vdev.vrings_num = 2;
    vdev.func = &dispatch;
    vdev.vrings_info = vrings;

    vrings[0].io = &vring_io[0];
    vrings[0].info.vaddr = (void*)VRING_TX_ADDRESS;
    vrings[0].info.align = VRING_ALIGN;
    vrings[0].info.num_descs = VRING_SIZE;
    vrings[0].info.pad = 0;
    vrings[0].vq = &vqs[0];

    vrings[1].io = &vring_io[1];
    vrings[1].info.vaddr = (void*)VRING_RX_ADDRESS;
    vrings[1].info.align = VRING_ALIGN;
    vrings[1].info.num_descs = VRING_SIZE;
    vrings[1].info.pad = 0;
    vrings[1].vq = &vqs[1];


    while (!(rsc_virtio_get_status(&vdev) & VIRTIO_CONFIG_STATUS_DRIVER_OK)) {
        for(int i=0; i<100000; i++) __asm__ volatile("nop");
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    rpmsg_init_vdev(&rvdev, &vdev, ns_bind_cb, &shm_io, &shpool);

    rpmsg_create_ept(&lept, &rvdev.rdev, "esphome-rpc", 1024, RPMSG_ADDR_ANY, endpoint_cb, endpoint_unbind_cb);
    uart_puts("Endpoint 'esphome-rpc' created!\r\n");

    rpmsg_create_ept(&rpc_ept, &rvdev.rdev, "smhub-rpc", 1025, RPMSG_ADDR_ANY, rpc_endpoint_cb, endpoint_unbind_cb);
    uart_puts("Endpoint 'smhub-rpc' created!\r\n");

    extern int request_irq(int irqn, int (*handler)(int, void*), unsigned long flags, const char *name, void *priv);
    request_irq(61, (int (*)(int, void*))mailbox_irq_handler, 0, "mailbox61", NULL);
}

extern volatile bool g_virtqueue_pending;
void rpmsg_send_keepalive(void);

void rpmsg_process_queue(void) {
    rpmsg_send_keepalive();
    
    if (g_virtqueue_pending) {
        g_virtqueue_pending = false;
        virtqueue_notification(&vqs[0]);
        virtqueue_notification(&vqs[1]);
    }
}

extern uint8_t connection_established;
static uint32_t last_rpmsg_ping = 0;
void rpmsg_send_keepalive(void) {
    if (!connection_established) {
        uint32_t now = xTaskGetTickCount();
        if (now - last_rpmsg_ping > pdMS_TO_TICKS(1000)) {
            uint8_t dummy_ping[] = {0x00, 0x00, 0x03};
            int ret = rpmsg_sendto(&lept, dummy_ping, sizeof(dummy_ping), 1024);
            if (ret < 0) uart_puts("Keepalive failed!\r\n"); else uart_puts("Keepalive sent!\r\n");
            last_rpmsg_ping = now;
        }
    }
}
extern void smhub_ipc_request_mac_address(void);
void rpmsg_request_mac(void) {
    smhub_ipc_request_mac_address();
}
extern void smhub_ipc_request_restart(void);
void rpmsg_request_restart(void) {
    smhub_ipc_request_restart();
}
