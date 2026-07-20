/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * remoteproc Resource Table layout & structures.
 */
#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Feature bits for VirtIO RPMSG
#define VIRTIO_ID_RPMSG_        7
#define VIRTIO_RPMSG_F_NS       0

// VirtIO Ring sizes (matches device tree 0x8000 size)
#define VRING_SIZE              256
#define VRING_ALIGN             4096

// Physical Addresses from Device Tree
#define VRING_TX_ADDRESS        0x8fc00000
#define VRING_RX_ADDRESS        0x8fc08000

// Resource table specific types
#include <openamp/remoteproc.h>

struct shared_resource_table {
    uint32_t version;
    uint32_t num;
    uint32_t reserved[2];
    uint32_t offset[1];
    struct fw_rsc_vdev vdev;
    struct fw_rsc_vdev_vring vrings[2];
} __attribute__((packed));

extern struct shared_resource_table resources;

#ifdef __cplusplus
}
#endif

#endif /* RSC_TABLE_H_ */
