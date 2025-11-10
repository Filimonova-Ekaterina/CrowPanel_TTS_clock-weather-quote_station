#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "crc_table.h"
#include "hm_comm_protocol.h"
#include "hm_regs.h"

#include <stddef.h>
#include <string.h>

static const char* TAG = "HM_COMM";

int hm_comm_reg_write_u8(hm_comm_transport_t* transport, uint8_t reg, const uint8_t data, uint32_t timeout)
{
    return hm_comm_reg_write(transport, reg, &data, 1, timeout);
}

int hm_comm_reg_write(hm_comm_transport_t* transport, uint8_t reg, const void* data, uint8_t len, uint32_t timeout)
{
    if (! data || len == 0)
        return HM_COMM_E_INV_ARG;

    ESP_LOGI(TAG, "write to reg %s %u bytes", hm_reg_to_str(reg), len);

    // total packet size
    size_t pkt_size = FRAME_PAYLOAD_OFFSET + len + CRC_BYTES;
    uint8_t packet[FRAME_MIN_SIZE + MAX_PAYLOAD_LEN];

    packet[FRAME_SOF_OFFSET]      = (uint8_t)SOF_VALUE;
    packet[FRAME_DEV_ADDR_OFFSET] = HM_DEV_ADDR_PACK(HM_DEV_ADDR, HM_DEV_RW_VAL_W);
    packet[FRAME_REG_OFFSET]      = reg;
    packet[FRAME_LEN_OFFSET]      = len;
    memcpy(&packet[FRAME_PAYLOAD_OFFSET], data, len);

    size_t crc_input_offset = FRAME_DEV_ADDR_OFFSET;
    size_t crc_input_len    = (FRAME_PAYLOAD_OFFSET - FRAME_DEV_ADDR_OFFSET) + len; // DEV_ADDR + REG + LEN + PAYLOAD
    uint16_t crc = crc16_compute(*get_crc16_lut(), get_crc16_config(), &packet[crc_input_offset], crc_input_len);
    hm_crc_to_bytes(crc, &packet[FRAME_PAYLOAD_OFFSET + len + 0], &packet[FRAME_PAYLOAD_OFFSET + len + 1]);

    int rc = transport->write(packet, pkt_size, timeout);
    return (rc >= 0) ? HM_COMM_E_OK : HM_COMM_E_FAIL;
}

int hm_comm_reg_read_u8(hm_comm_transport_t* transport, uint8_t reg, uint8_t* data, uint32_t timeout)
{
    return hm_comm_reg_read(transport, reg, data, 1, timeout);
}

int hm_comm_reg_read(hm_comm_transport_t* transport, uint8_t reg, void* data, uint8_t len, uint32_t timeout)
{
    if (! data || len == 0)
        return HM_COMM_E_INV_ARG;

    ESP_LOGI(TAG, "read reg %s %u bytes", hm_reg_to_str(reg), len);

    // Build request packet
    uint8_t req[FRAME_MIN_SIZE];
    req[FRAME_SOF_OFFSET]      = (uint8_t)SOF_VALUE;
    req[FRAME_DEV_ADDR_OFFSET] = HM_DEV_ADDR_PACK(HM_DEV_ADDR, HM_DEV_RW_VAL_R);
    req[FRAME_REG_OFFSET]      = reg;
    req[FRAME_LEN_OFFSET]      = len; // request that many bytes

    size_t crc_input_offset = FRAME_DEV_ADDR_OFFSET;
    size_t crc_input_len    = HEADER_BYTES; // DEV_ADDR + REG + LEN
    uint16_t crc = crc16_compute(*get_crc16_lut(), get_crc16_config(), &req[crc_input_offset], crc_input_len);
    hm_crc_to_bytes(crc, &req[FRAME_DEV_ADDR_OFFSET + HEADER_BYTES + 0],
                    &req[FRAME_DEV_ADDR_OFFSET + HEADER_BYTES + 1]);

    // send request
    int rc = transport->write(req, FRAME_MIN_SIZE, timeout);
    if (rc < 0)
        return HM_COMM_E_FAIL; // send failed

    // now receive response: wait for SOF, then read header, payload and CRC
    uint8_t byte;
    int64_t start_time = esp_timer_get_time(); // microseconds
    int64_t timeout_us = timeout * 1000LL;

    // find SOF
    while (1) {
        // Check for timeout
        int64_t elapsed_time = esp_timer_get_time() - start_time;
        if (elapsed_time > timeout_us) {
            return HM_COMM_E_TIMEOUT;
        }
        int r = transport->read(&byte, 1, 0);
        if (r < 0)
            return HM_COMM_E_FAIL;
        if (r > 0) {
            if (byte == (uint8_t)SOF_VALUE)
                break;
            else
                ESP_LOGV(TAG, "sof read error: %d", r);
        }
    }

    uint8_t recv_buffer[HEADER_BYTES + MAX_PAYLOAD_LEN];
    int r = transport->read(recv_buffer, HEADER_BYTES, timeout);
    if (r != (int)HEADER_BYTES) {
        ESP_LOGE(TAG, "header read error: %d", r);
        return HM_COMM_E_TIMEOUT;
    }

    uint8_t resp_addr = recv_buffer[0];
    uint8_t resp_reg  = recv_buffer[1];
    uint8_t resp_len  = recv_buffer[2];

    uint8_t rw   = HM_DEV_ADDR_RW(resp_addr);
    uint8_t addr = HM_DEV_ADDR_ADDR(resp_addr);
    ESP_LOGD(TAG, "recv frame: addr=0x%x, rw=%u, reg=0x%x, payload_len=%u", addr, rw, resp_reg, resp_len);
    if (addr != HM_DEV_ADDR) { // wrong device address
        ESP_LOGE(TAG, "incorrect address");
        return HM_COMM_E_DEV_ADDR;
    }
    if (resp_reg != reg) {
        ESP_LOGE(TAG, "incorrect register");
        return HM_COMM_E_FAIL; // wrong register returned
    }
    if (resp_len != len) {
        ESP_LOGE(TAG, "incorrect payload length: %u", resp_len);
        return HM_COMM_E_FAIL; // unexpected length
    }

    // read payload
    uint8_t* payload_ptr = &recv_buffer[HEADER_BYTES];
    int read_bytes       = transport->read(payload_ptr, resp_len, timeout);
    if (read_bytes != resp_len) {
        ESP_LOGE(TAG, "payload read error: %d", read_bytes);
        return HM_COMM_E_TIMEOUT;
    }

    // read CRC
    uint8_t crc_bytes[CRC_BYTES];
    r = transport->read(crc_bytes, CRC_BYTES, timeout);
    if (r != CRC_BYTES) {
        ESP_LOGE(TAG, "unable to read crc bytes: %d", r);
        return HM_COMM_E_TIMEOUT;
    }

    // verify CRC (computed over DEV_ADD..PAYLOAD)
    uint16_t crc_calc = crc16_compute(*get_crc16_lut(), get_crc16_config(), recv_buffer, HEADER_BYTES + resp_len);
    uint16_t crc_recv = hm_crc_from_bytes(crc_bytes[0], crc_bytes[1]);
    if (crc_calc != crc_recv) {
        ESP_LOGE(TAG, "wrong crc: crc_calc=%u, crc_recv=%u", crc_calc, crc_recv);
        return HM_COMM_E_CRC;
    }

    memcpy(data, payload_ptr, read_bytes);

    return HM_COMM_E_OK;
}