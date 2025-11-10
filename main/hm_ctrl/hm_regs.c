#include "hm_regs.h"

#define STRINGIFY(name) #name
#define TOSTRING(x)     STRINGIFY(x)

#define REG_NAME(name) [name##_ADDR] = TOSTRING(name)

static const char* reg_names[HM_REG_MAP_SIZE] = {
    REG_NAME(HM_REG_VERSION),
    REG_NAME(HM_REG_STATUS),
    REG_NAME(HM_REG_ERR),
    REG_NAME(HM_REG_CMD),
    REG_NAME(HM_REG_TXT_FORMAT),
    REG_NAME(HM_REG_SPK_GAIN),
    REG_NAME(HM_REG_FLAGS),
    REG_NAME(HM_REG_BUFFER_LEN),
    REG_NAME(HM_REG_BUFFER_POS),
    REG_NAME(HM_REG_BUFFER_STATUS),
    REG_NAME(HM_REG_BUFFER_DATA),
    REG_NAME(HM_REG_INT_MASK),
    REG_NAME(HM_REG_INT_STATUS),
};

const char* hm_reg_to_str(uint8_t reg)
{
    if (reg >= HM_REG_MAP_SIZE)
        return "";
    return reg_names[reg];
}

const char* hm_status_to_str(hm_status_t status)
{
    switch (status) {
    case HM_STATUS_READY:
        return "HM_STATUS_READY";
    case HM_STATUS_BUSY:
        return "HM_STATUS_BUSY";
        break;
    case HM_STATUS_PAUSED:
        return "HM_STATUS_PAUSED";
    default:
        return "";
    }
}

const char* hm_dev_cmd_to_str(hm_dev_cmd_t cmd)
{
    switch (cmd) {
    case HM_DEV_CMD_NONE:
        return "HM_DEV_CMD_NONE";
    case HM_DEV_CMD_START:
        return "HM_DEV_CMD_START";
    case HM_DEV_CMD_STOP:
        return "HM_DEV_CMD_STOP";
    case HM_DEV_CMD_PAUSE:
        return "HM_DEV_CMD_PAUSE";
    case HM_DEV_CMD_RESUME:
        return "HM_DEV_CMD_RESUME";
    case HM_DEV_CMD_RESET:
        return "HM_DEV_CMD_RESET";
    case HM_DEV_CMD_FULL_RESET:
        return "HM_DEV_CMD_FULL_RESET";
    case HM_BUFFER_CMD_ALLOCATE:
        return "HM_BUFFER_CMD_ALLOCATE";
    case HM_BUFFER_CMD_RESET:
        return "HM_BUFFER_CMD_RESET";
    default:
        return "";
    }
}

const char* hm_buffer_status_to_str(hm_buffer_status_t buffer_status)
{
    switch (buffer_status) {
    case HM_BUFFER_STATUS_EMPTY:
        return "HM_BUFFER_STATUS_EMPTY";
    case HM_BUFFER_STATUS_READY:
        return "HM_BUFFER_STATUS_READY";
    case HM_BUFFER_STATUS_LOCKED:
        return "HM_BUFFER_STATUS_LOCKED";
    default:
        return "";
    }
}

const char* hm_err_to_str(hm_err_t err)
{
    switch (err) {
    case HM_ERR_OK:
        return "HM_ERR_OK";
    case HM_ERR_CRC:
        return "HM_ERR_CRC";
    case HM_ERR_NOT_READY:
        return "HM_ERR_NOT_READY";
    case HM_ERR_BUFFER_OVERFLOW:
        return "HM_ERR_BUFFER_OVERFLOW";
    case HM_ERR_BUFFER_NO_MEM:
        return "HM_ERR_BUFFER_NO_MEM";
    case HM_ERR_REG_INVALID_ADDR:
        return "HM_ERR_REG_INVALID_ADDR";
    case HM_ERR_REG_READ_NOT_ALLOWED:
        return "HM_ERR_REG_READ_NOT_ALLOWED";
    case HM_ERR_REG_WRITE_NOT_ALLOWED:
        return "HM_ERR_REG_WRITE_NOT_ALLOWED";
    case HM_ERR_REG_OUT_OF_BOUNDS:
        return "HM_ERR_REG_OUT_OF_BOUNDS";
    case HM_ERR_REG_INV_VAL:
        return "HM_ERR_REG_INV_VAL";
    case HM_ERR_GEN_ERROR:
        return "HM_ERR_GEN_ERROR";
    case HM_ERR_UNKNOWN:
        return "HM_ERR_UNKNOWN";
    default:
        return "";
    }
}
