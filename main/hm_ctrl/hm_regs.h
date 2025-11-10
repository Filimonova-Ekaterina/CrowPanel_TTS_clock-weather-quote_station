#ifndef HM_REGS_H_
#define HM_REGS_H_

#include <stdint.h>

/* dev status codes */
typedef enum {
    HM_STATUS_READY = 0,
    HM_STATUS_BUSY,
    HM_STATUS_PAUSED,
    HM_STATUS_MAX = 0xff
} hm_status_t;

/* cmd codes */
typedef enum {
    HM_DEV_CMD_NONE = 0,
    HM_DEV_CMD_START,
    HM_DEV_CMD_STOP,
    HM_DEV_CMD_PAUSE,
    HM_DEV_CMD_RESUME,
    HM_DEV_CMD_RESET,
    HM_DEV_CMD_FULL_RESET,
    HM_BUFFER_CMD_ALLOCATE,
    HM_BUFFER_CMD_RESET,
    HM_DEV_CMD_MAX = 0xff
} hm_dev_cmd_t;

/* buffer status codes */
typedef enum {
    HM_BUFFER_STATUS_EMPTY = 0,
    HM_BUFFER_STATUS_READY,
    HM_BUFFER_STATUS_LOCKED,
    HM_BUFFER_STATUS_MAX = 0xff
} hm_buffer_status_t;

/* error codes */
typedef enum {
    HM_ERR_OK = 0,
    HM_ERR_CRC,
    HM_ERR_NOT_READY,
    HM_ERR_BUFFER_OVERFLOW,
    HM_ERR_BUFFER_NO_MEM,
    HM_ERR_REG_INVALID_ADDR,
    HM_ERR_REG_READ_NOT_ALLOWED,
    HM_ERR_REG_WRITE_NOT_ALLOWED,
    HM_ERR_REG_OUT_OF_BOUNDS,
    HM_ERR_REG_INV_VAL,
    HM_ERR_GEN_ERROR,
    HM_ERR_UNKNOWN,
    HM_ERR_MAX = 0xff
} hm_err_t;

/* property masks */
#define HM_REG_PROP_R    ((uint8_t)(1U << 0))
#define HM_REG_PROP_W    ((uint8_t)(1U << 1))
#define HM_REG_PROP_RCLR ((uint8_t)(1U << 2))
#define HM_REG_PROP_RW   (HM_REG_PROP_R | HM_REG_PROP_W)

/* version pack/unpack helpers */
#define HM_REG_VERSION_PACK(maj, min, patch)                                                                           \
    ((((uint32_t)(maj) & 0xffU) << 24) | (((uint32_t)(min) & 0xffU) << 16) | (((uint32_t)(patch) & 0xffffU)))

#define HM_REG_VERSION_MAJOR(v) ((uint8_t)(((v) >> 24) & 0xffU))
#define HM_REG_VERSION_MINOR(v) ((uint8_t)(((v) >> 16) & 0xffU))
#define HM_REG_VERSION_PATCH(v) ((uint16_t)((v) & 0xffffU))

/* flags bit masks */
#define HM_REG_FLAGS_REPEAT_OFFSET  (0U)
#define HM_REG_FLAGS_REPEAT_MSK     ((uint8_t)(1U << HM_REG_FLAGS_REPEAT_OFFSET))
#define HM_REG_FLAGS_REPEAT_VAL_DIS ((uint8_t)0U)
#define HM_REG_FLAGS_REPEAT_VAL_EN  ((uint8_t)1U)
#define HM_REG_FLAGS_REPEAT_VAL_DEF HM_REG_FLAGS_REPEAT_VAL_DIS

#define HM_REG_FLAGS_BUFFER_RESET_ON_DONE_OFFSET (1U)
#define HM_REG_FLAGS_BUFFER_RESET_ON_DONE_MSK    ((uint8_t)(1U << HM_REG_FLAGS_BUFFER_RESET_ON_DONE_OFFSET))
#define HM_REG_FLAGS_BUFFER_RESET_VAL_DIS        ((uint8_t)0U)
#define HM_REG_FLAGS_BUFFER_RESET_VAL_EN         ((uint8_t)1U)
#define HM_REG_FLAGS_BUFFER_RESET_VAL_DEF        HM_REG_FLAGS_BUFFER_RESET_VAL_DIS

/* flags pack/unpack helpers */
#define HM_REG_FLAGS_PACK(repeat_val, reset_on_done)                                                                   \
    ((uint8_t)(((repeat_val) & 0x1U) << HM_REG_FLAGS_REPEAT_OFFSET) |                                                  \
     ((uint8_t)(((reset_on_done) & 0x1U) << HM_REG_FLAGS_BUFFER_RESET_ON_DONE_OFFSET)))
#define HM_REG_FLAGS_REPEAT(byte) (((byte) & HM_REG_FLAGS_REPEAT_MSK) >> HM_REG_FLAGS_REPEAT_OFFSET)
#define HM_REG_FLAGS_BUFFER_RESET_ON_DONE(byte)                                                                        \
    (((byte) & HM_REG_FLAGS_BUFFER_RESET_ON_DONE_MSK) >> HM_REG_FLAGS_BUFFER_RESET_ON_DONE_OFFSET)

#define HM_REG_INT_MASK_DONE_OFFSET  (0U)
#define HM_REG_INT_MASK_DONE_MSK     ((uint8_t)(1U << HM_REG_INT_MASK_DONE_OFFSET))
#define HM_REG_INT_MASK_ERROR_OFFSET (1U)
#define HM_REG_INT_MASK_ERROR_MSK    ((uint8_t)(1U << HM_REG_INT_MASK_ERROR_OFFSET))

#define HM_REG_INT_STATUS_DONE_MSK  HM_REG_INT_MASK_DONE_MSK
#define HM_REG_INT_STATUS_ERROR_MSK HM_REG_INT_MASK_ERROR_MSK

/* int mask pack/unpack helpers */
#define HM_REG_INT_MASK_PACK(done, error)                                                                              \
    ((uint8_t)(((done) & 0x1U) << HM_REG_INT_MASK_DONE_OFFSET)) |                                                      \
        ((uint8_t)(((error) & 0x1U) << HM_REG_INT_MASK_ERROR_OFFSET))

#define REG_DEF(name, addr, width, props, defval, mask) \
    enum { name##_ADDR = addr };                        \
    enum { name##_WIDTH = width  };                     \
    enum { name##_PROPS = props  };                     \
    enum { name##_DEFAULT = defval };                   \
    enum { name##_MASK = mask };

REG_DEF(HM_REG_VERSION,       0x00, 4, HM_REG_PROP_R,                    0x00, 0xffffffff)
REG_DEF(HM_REG_STATUS,        0x04, 1, HM_REG_PROP_R,                    HM_STATUS_READY, 0xff)
REG_DEF(HM_REG_ERR,           0x05, 1, HM_REG_PROP_R | HM_REG_PROP_RCLR, HM_ERR_OK, 0xff)
REG_DEF(HM_REG_CMD,           0x06, 1, HM_REG_PROP_W,                    HM_DEV_CMD_NONE, 0xff)
REG_DEF(HM_REG_TXT_FORMAT,    0x07, 1, HM_REG_PROP_RW,                   0x00, 0xff)
REG_DEF(HM_REG_SPK_GAIN,      0x08, 1, HM_REG_PROP_RW,                   0x00, 0xff)
REG_DEF(HM_REG_FLAGS,         0x09, 1, HM_REG_PROP_RW,                   HM_REG_FLAGS_PACK(HM_REG_FLAGS_REPEAT_VAL_DEF, HM_REG_FLAGS_BUFFER_RESET_VAL_DEF), 0xff)
REG_DEF(HM_REG_BUFFER_LEN,    0x0a, 2, HM_REG_PROP_RW,                   0x00, 0xffff)
REG_DEF(HM_REG_BUFFER_POS,    0x0c, 2, HM_REG_PROP_RW,                   0x00, 0xffff)
REG_DEF(HM_REG_BUFFER_STATUS, 0x0e, 1, HM_REG_PROP_R,                    HM_BUFFER_STATUS_EMPTY, 0xff)
REG_DEF(HM_REG_BUFFER_DATA,   0x0f, 1, HM_REG_PROP_W,                    0x00, 0x00)
REG_DEF(HM_REG_INT_MASK,      0x10, 1, HM_REG_PROP_RW,                   0x00, 0x00)
REG_DEF(HM_REG_INT_STATUS,    0x11, 1, HM_REG_PROP_R | HM_REG_PROP_RCLR, 0x00, 0x00)

#define HM_REG_MAP_SIZE (HM_REG_INT_STATUS_ADDR + 1)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const char* hm_reg_to_str(uint8_t reg);
const char* hm_status_to_str(hm_status_t status);
const char* hm_dev_cmd_to_str(hm_dev_cmd_t cmd);
const char* hm_buffer_status_to_str(hm_buffer_status_t buffer_status);
const char* hm_err_to_str(hm_err_t err);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // HM_REGS_H_