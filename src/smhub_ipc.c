/*
 * Copyright (c) 2026 SMLIGHT
 * All rights reserved.
 * 
 * Inter-Processor Communication (IPC) layers for Protobuf-RPC.
 */
#include "smhub_ipc.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <string.h>

extern void rpc_rpmsg_tx(const uint8_t *data, size_t len);

bool smhub_ipc_send_rpc(const smhub_hal_rpc_RpcCommand *cmd) {
    uint8_t buffer[512]; 
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    
    if (!pb_encode(&stream, smhub_hal_rpc_RpcCommand_fields, cmd)) {
        return false;
    }
    
    rpc_rpmsg_tx(buffer, stream.bytes_written);
    return true;
}

extern void uart_puts(const char*);

void smhub_ipc_handle_rx(const uint8_t *payload, size_t len) {
    pb_istream_t stream = pb_istream_from_buffer(payload, len);
    smhub_hal_rpc_RpcCommand cmd = smhub_hal_rpc_RpcCommand_init_zero;
    
    if (pb_decode(&stream, smhub_hal_rpc_RpcCommand_fields, &cmd)) {
        switch(cmd.type) {
            case smhub_hal_rpc_CommandType_GET_ENTROPY_RESP:
                extern void esphome_rpmsg_entropy_response_pb(const smhub_hal_rpc_GetEntropyResp *resp);
                esphome_rpmsg_entropy_response_pb(&cmd.entropy_resp);
                break;
            case smhub_hal_rpc_CommandType_GPIO_EDGE_EVENT:
                // Extern call into ESPHome GPIO component
                extern void smhub_ipc_gpio_edge_cb(const char* pin_name, bool state);
                smhub_ipc_gpio_edge_cb(cmd.gpio_edge.pin_name, cmd.gpio_edge.new_state);
                break;
            case smhub_hal_rpc_CommandType_MAC_ADDRESS_RESP:
                uart_puts("smhub_ipc_handle_rx: MAC_ADDRESS_RESP received!\r\n");
                // Extern call into ESPHome core to inject MAC
                extern void sg2000_set_mac(const uint8_t* mac) __attribute__((weak));
                if (sg2000_set_mac) {
                    if (cmd.mac_address_resp.mac.size == 6) {
                        sg2000_set_mac(cmd.mac_address_resp.mac.bytes);
                        uart_puts("smhub_ipc_handle_rx: sg2000_set_mac called!\r\n");
                    } else {
                        uart_puts("smhub_ipc_handle_rx: MAC size != 6!\r\n");
                    }
                } else {
                    uart_puts("smhub_ipc_handle_rx: sg2000_set_mac is NULL!\r\n");
                }
                break;
            case smhub_hal_rpc_CommandType_PREF_LOAD_RESP:
                extern void smhub_ipc_pref_load_cb(uint32_t hash, const uint8_t *data, size_t len) __attribute__((weak));
                if (smhub_ipc_pref_load_cb) {
                    smhub_ipc_pref_load_cb(cmd.pref_load_resp.hash, cmd.pref_load_resp.data.bytes, cmd.pref_load_resp.data.size);
                }
                break;
            case smhub_hal_rpc_CommandType_GET_TIME_RESP:
                extern void smhub_ipc_time_cb(uint64_t epoch_seconds) __attribute__((weak));
                if (smhub_ipc_time_cb) {
                    smhub_ipc_time_cb(cmd.time_resp.epoch_seconds);
                }
                break;
            case smhub_hal_rpc_CommandType_SWITCH_CONTROL:
                if (cmd.has_switch_req) {
                    extern void smhub_rpc_control_switch(uint32_t key, bool state);
                    smhub_rpc_control_switch(cmd.switch_req.key, cmd.switch_req.state);
                }
                break;
            case smhub_hal_rpc_CommandType_LIGHT_CONTROL:
                if (cmd.has_light_req) {
                    extern void smhub_rpc_control_light(uint32_t key, bool state, float brightness, float r, float g, float b, const char* effect);
                    smhub_rpc_control_light(cmd.light_req.key, cmd.light_req.state, 
                                            cmd.light_req.has_brightness ? cmd.light_req.brightness : 1.0f,
                                            cmd.light_req.has_rgb ? cmd.light_req.red : 1.0f,
                                            cmd.light_req.has_rgb ? cmd.light_req.green : 1.0f,
                                            cmd.light_req.has_rgb ? cmd.light_req.blue : 1.0f,
                                            cmd.light_req.effect);
                }
                break;
            case smhub_hal_rpc_CommandType_BUZZER_CONTROL:
                if (cmd.has_buzzer_req) {
                    extern void smhub_rpc_control_buzzer(const char* song);
                    smhub_rpc_control_buzzer(cmd.buzzer_req.song);
                }
                break;
            // Additional routing here as we migrate more endpoints
            default:
                break;
        }
    }
}

void smhub_ipc_request_restart(void) {
    smhub_hal_rpc_RpcCommand cmd = smhub_hal_rpc_RpcCommand_init_zero;
    cmd.type = smhub_hal_rpc_CommandType_RESTART_REQ;
    smhub_ipc_send_rpc(&cmd);
}

void smhub_ipc_request_mac_address(void) {
    smhub_hal_rpc_RpcCommand cmd = smhub_hal_rpc_RpcCommand_init_zero;
    cmd.type = smhub_hal_rpc_CommandType_MAC_ADDRESS_REQ;
    smhub_ipc_send_rpc(&cmd);
}
