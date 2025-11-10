#ifndef HM_COMM_PROTOCOL_H_
#define HM_COMM_PROTOCOL_H_

#include <stdint.h>

#include "hm_comm_protocol_def.h"

enum hm_comm_rc {
    HM_COMM_E_OK       = 0,
    HM_COMM_E_FAIL     = -1,
    HM_COMM_E_INV_ARG  = -2,
    HM_COMM_E_TIMEOUT  = -3,
    HM_COMM_E_CRC      = -4,
    HM_COMM_E_DEV_ADDR = -5
};

typedef struct {
    int (*init)(void);
    int (*deinit)(void);
    int (*flush)(void);
    int (*write)(void* data, uint32_t bytes, uint32_t timeout);
    int (*read)(void* data, uint32_t bytes, uint32_t timeout);
} hm_comm_transport_t;

typedef struct {
    uint8_t reg;
    uint8_t rw;
    struct {
        uint8_t len;
        union {
            const uint8_t* payload;
            uint8_t* scratch;
        };
    } buffer;
} hm_comm_parsed_frame_t;

typedef int (*hm_comm_register_request_handler_t)(const hm_comm_parsed_frame_t* frame);

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int hm_comm_reg_write(hm_comm_transport_t* transport, uint8_t reg, const void* data, uint8_t len, uint32_t timeout);
int hm_comm_reg_write_u8(hm_comm_transport_t* transport, uint8_t reg, const uint8_t byte, uint32_t timeout);
int hm_comm_reg_read(hm_comm_transport_t* transport, uint8_t reg, void* data, uint8_t len, uint32_t timeout);
int hm_comm_reg_read_u8(hm_comm_transport_t* transport, uint8_t reg, uint8_t* byte, uint32_t timeout);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // HM_COMM_PROTOCOL_H_
