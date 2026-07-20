/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * SMLIGHT IPC Protocol interface definitions.
 */
#ifndef SMHUB_IPC_H
#define SMHUB_IPC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "gen/rpc.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Generic IPC Sender for pure RpcCommand
bool smhub_ipc_send_rpc(const smhub_hal_rpc_RpcCommand *cmd);

// Decodes incoming VirtIO payload and dispatches
void smhub_ipc_handle_rx(const uint8_t *payload, size_t len);

// System Control
void smhub_ipc_request_restart(void);
void smhub_ipc_request_mac_address(void);

// Extern callback for handling entropy (implemented in newlib_freertos.c)
void esphome_rpmsg_entropy_response_pb(const smhub_hal_rpc_GetEntropyResp *resp);

#ifdef __cplusplus
}
#endif

#endif
