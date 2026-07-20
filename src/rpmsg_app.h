/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * RPMsg definitions and application helper hooks.
 */
#ifndef RPMSG_APP_H
#define RPMSG_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void rpmsg_app_init(void);
void rpmsg_process_queue(void);
bool rpmsg_mac_received(void);
void rpmsg_request_mac(void);

#ifdef __cplusplus
}
#endif

#endif
