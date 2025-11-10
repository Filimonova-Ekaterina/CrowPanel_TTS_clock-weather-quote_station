#ifndef _CRC_TABLE_H_
#define _CRC_TABLE_H_

#include "crc.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

const crc16_config_t* get_crc16_config();
const crc16_table* get_crc16_lut();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _CRC_TABLE_H_
