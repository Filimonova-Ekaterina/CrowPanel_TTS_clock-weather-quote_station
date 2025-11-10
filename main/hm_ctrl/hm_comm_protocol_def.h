#ifndef HM_COMM_PROTOCOL_DEF_H_
#define HM_COMM_PROTOCOL_DEF_H_

#include <stdint.h>

#define HM_DEV_ADDR 0x24U

// Frame layout: [SOF] [DEV_ADDR] [REG] [LEN] [PAYLOAD...] [CRC_H] [CRC_L]
// SOF      = 1 byte (SOF_VALUE)
// DEV_ADDR = 1 byte dev address
// REG      = 1 byte register
// LEN      = 1 byte payload length (0..MAX_PAYLOAD_LEN)

#define SOF_VALUE         0x01U
#define SOF_BYTES         1
#define DEV_ADDR_BYTES    1
#define REG_BYTES         1
#define PAYLOAD_LEN_BYTES 1
#define CRC_BYTES         2
#define HEADER_BYTES      (DEV_ADDR_BYTES + REG_BYTES + PAYLOAD_LEN_BYTES)

#define MAX_PAYLOAD_LEN 255U

// offsets from start of frame
#define FRAME_SOF_OFFSET      0
#define FRAME_DEV_ADDR_OFFSET (FRAME_SOF_OFFSET + SOF_BYTES)
#define FRAME_REG_OFFSET      (FRAME_DEV_ADDR_OFFSET + DEV_ADDR_BYTES)
#define FRAME_LEN_OFFSET      (FRAME_REG_OFFSET + REG_BYTES)
#define FRAME_PAYLOAD_OFFSET  (FRAME_LEN_OFFSET + PAYLOAD_LEN_BYTES)

// Full minimum frame size (no payload)
#define FRAME_MIN_SIZE (SOF_BYTES + DEV_ADDR_BYTES + REG_BYTES + PAYLOAD_LEN_BYTES + CRC_BYTES)

/* CRC value helpers */
static inline void hm_crc_to_bytes(uint16_t crc, uint8_t* h, uint8_t* l)
{
    *h = (uint8_t)(crc >> 8); /* Big-endian on wire: CRC_H then CRC_L */
    *l = (uint8_t)(crc & 0xFF);
}
static inline uint16_t hm_crc_from_bytes(uint8_t h, uint8_t l) { return (uint16_t)h << 8 | (uint16_t)l; }

/* DEV addressing is similar to I2C */
#define HM_DEV_ADDR_OFFSET  (1U)
#define HM_DEV_ADDR_MSK     ((uint8_t)(0x7fU << HM_DEV_ADDR_OFFSET))
#define HM_DEV_ADDR_VAL_DEF ((uint8_t)0U)

#define HM_DEV_RW_BIT_OFFSET (0U)
#define HM_DEV_RW_BIT_MSK    ((uint8_t)(1U << HM_DEV_RW_BIT_OFFSET))
#define HM_DEV_RW_VAL_W      ((uint8_t)0U)
#define HM_DEV_RW_VAL_R      ((uint8_t)1U)
#define HM_DEV_RW_VAL_DEF    HM_DEV_RW_VAL_W

#define HM_DEV_ADDR_PACK(addr7, rw) ((uint8_t)(((addr7) << HM_DEV_ADDR_OFFSET) | ((rw) << HM_DEV_RW_BIT_OFFSET)))
#define HM_DEV_ADDR_ADDR(addr_byte) ((uint8_t)(((addr_byte) & HM_DEV_ADDR_MSK) >> HM_DEV_ADDR_OFFSET))
#define HM_DEV_ADDR_RW(addr_byte)   ((uint8_t)(((addr_byte) & HM_DEV_RW_BIT_MSK) >> HM_DEV_RW_BIT_OFFSET))

#endif // HM_COMM_PROTOCOL_DEF_H_
