/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * remoteproc Resource Table configuration.
 */
#include "rsc_table.h"

// Put the resource table in a dedicated section so the linker script can place it
// at the beginning of the firmware image, making it easily discoverable by remoteproc.
__attribute__((section(".resource_table")))
struct shared_resource_table resources = {
    .version = 1,
    .num = 1,
    .reserved = {0, 0},
    .offset = {
        offsetof(struct shared_resource_table, vdev),
    },
    
    // VirtIO Device Entry
    .vdev = {
        .type = RSC_VDEV,
        .id = VIRTIO_ID_RPMSG_,
        .notifyid = 0,
        .dfeatures = (1 << VIRTIO_RPMSG_F_NS), // Support Name Service
        .gfeatures = 0,
        .config_len = 0,
        .status = 0,
        .num_of_vrings = 2,
        .reserved = {0, 0},
    },
    .vrings = {
        // VRING 0 (TX)
        {
            .da = VRING_TX_ADDRESS,
            .align = VRING_ALIGN,
            .num = VRING_SIZE,
            .notifyid = 0, // Mailbox notify ID for TX
            .reserved = 0
        },
        // VRING 1 (RX)
        {
            .da = VRING_RX_ADDRESS,
            .align = VRING_ALIGN,
            .num = VRING_SIZE,
            .notifyid = 1, // Mailbox notify ID for RX
            .reserved = 0
        }
    }
};
