#include "freertos/FreeRTOS.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "string.h"

#include "HxTTS.h"
#include "uart.h"

extern "C" {
#include "ui_events.h"   
}

#define HM_INT_GPIO_PIN GPIO_NUM_2

#define CHECK_COMM_CALL(st)                                                                                            \
    do {                                                                                                               \
        int ret = st;                                                                                                  \
        if (ret != HM_COMM_E_OK) {                                                                                     \
            ESP_LOGE(TAG, "TIMEOUT in %s", __FUNCTION__);                                                              \
            return HxTTS::TIMEOUT;                                                                                     \
        }                                                                                                              \
    } while (0);

static void tts_monitor_task(void *arg)
{
    HxTTS *tts = reinterpret_cast<HxTTS*>(arg);
    if (!tts) {
        vTaskDelete(NULL);
        return;
    }

    HxTTS::Error err = tts->waitReady(60000);
    if (err != HxTTS::Error::OK) {
        ESP_LOGW("HxTTS", "waitReady returned %d", static_cast<int>(err));
    }

    ui_notify_tts_finished();

   
    vTaskDelete(NULL);
}


HxTTS::HxTTS(BusType bus_type)
{
    switch (bus_type) {
    case BusType::UART:
    default:
        transport = {
            .init   = uart_init,
            .deinit = uart_release,
            .flush  = uart_flush_buffers,
            .write  = uart_write,
            .read   = uart_read,
        };
        break;
    }

    transport.init();

    gpio_config_t gpio_conf = {.pin_bit_mask = (1ULL << HM_INT_GPIO_PIN),
                               .mode         = GPIO_MODE_INPUT,
                               .pull_up_en   = GPIO_PULLUP_DISABLE,
                               .pull_down_en = GPIO_PULLDOWN_ENABLE,
                               .intr_type    = GPIO_INTR_DISABLE};
    gpio_config(&gpio_conf);

    uint8_t int_status;
    hm_comm_reg_read_u8(&transport, HM_REG_INT_STATUS_ADDR, &int_status, HX_REQ_TIMEOUT_MS);
    ESP_LOGI(TAG, "int_status=%u", int_status);
    hm_comm_reg_write_u8(&transport, HM_REG_INT_MASK_ADDR, HM_REG_INT_MASK_PACK(1, 0), HX_REQ_TIMEOUT_MS);
}
HxTTS::~HxTTS() { transport.deinit(); }

HxTTS::Error HxTTS::getVersion(int& major, int& minor, int& patch)
{
    uint32_t version;
    CHECK_COMM_CALL(hm_comm_reg_read(&transport, HM_REG_VERSION_ADDR, &version, 4, HX_REQ_TIMEOUT_MS));
    major = HM_REG_VERSION_MAJOR(version);
    minor = HM_REG_VERSION_MINOR(version);
    patch = HM_REG_VERSION_PATCH(version);
    return Error::OK;
}

HxTTS::Error HxTTS::getStatus(hm_status_t& status)
{
    uint8_t status_byte = 0xff;
    CHECK_COMM_CALL(hm_comm_reg_read_u8(&transport, HM_REG_STATUS_ADDR, &status_byte, HX_REQ_TIMEOUT_MS));
    ESP_LOGD(TAG, "status_byte=%u", status_byte);
    status = (hm_status_t)(status_byte & HM_REG_STATUS_MASK);
    return Error::OK;
}

HxTTS::Error HxTTS::getError(hm_err_t& error)
{
    uint8_t error_byte = 0xff;
    CHECK_COMM_CALL(hm_comm_reg_read_u8(&transport, HM_REG_ERR_ADDR, &error_byte, HX_REQ_TIMEOUT_MS));
    ESP_LOGD(TAG, "error_byte=%u", error_byte);
    error = (hm_err_t)(error_byte & HM_REG_ERR_MASK);
    return Error::OK;
}

HxTTS::Error HxTTS::startPlayback()
{
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_START, HX_REQ_TIMEOUT_MS));

    
    BaseType_t r = xTaskCreate(
        tts_monitor_task,    
        "tts_mon",           
        4096,                
        this,                
        tskIDLE_PRIORITY + 3,
        NULL                 
    );
    if (r != pdPASS) {
        ESP_LOGW(TAG, "Failed to create tts_monitor_task");
        
    }

    return Error::OK;
}


HxTTS::Error HxTTS::stopPlayback()
{
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_STOP, HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::pausePlayback()
{
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_PAUSE, HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::resumePlayback()
{
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_RESUME, HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::waitReady(uint32_t timeout)
{
    
    hm_status_t status;
    uint32_t elapsed                         = 0;
    static constexpr uint32_t polling_period = pdMS_TO_TICKS(100);
    do {
        Error ret = getStatus(status);
        if (ret != Error::OK) {
            return ret;
        }
        ESP_LOGI(TAG, "status=%s", hm_status_to_str(status));
        vTaskDelay(polling_period);
        elapsed += polling_period;
        if (elapsed > timeout) {
            return Error::TIMEOUT;
        }
    } while (status == HM_STATUS_BUSY);
    return Error::OK;
}

HxTTS::Error HxTTS::reset(bool full)
{
    if (full) {
        CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_FULL_RESET, HX_REQ_TIMEOUT_MS));
    } else {
        CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_DEV_CMD_RESET, HX_REQ_TIMEOUT_MS));
    }
    return Error::OK;
}

HxTTS::Error HxTTS::setRepeatMode(bool value)
{
    CHECK_COMM_CALL(
        hm_comm_reg_write_u8(&transport, HM_REG_FLAGS_ADDR, HM_REG_FLAGS_PACK(value, 0), HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::increaseVolume()
{
    uint8_t volume;
    CHECK_COMM_CALL(hm_comm_reg_read_u8(&transport, HM_REG_SPK_GAIN_ADDR, &volume, HX_REQ_TIMEOUT_MS));
    ESP_LOGD(TAG, "current volume=%u", volume);
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_SPK_GAIN_ADDR, volume == 11 ? volume : (volume + 1),
                                         HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::decreaseVolume()
{
    uint8_t volume;
    CHECK_COMM_CALL(hm_comm_reg_read_u8(&transport, HM_REG_SPK_GAIN_ADDR, &volume, HX_REQ_TIMEOUT_MS));
    ESP_LOGI(TAG, "current volume=%u", volume);
    CHECK_COMM_CALL(
        hm_comm_reg_write_u8(&transport, HM_REG_SPK_GAIN_ADDR, volume == 0 ? volume : (volume - 1), HX_REQ_TIMEOUT_MS));
    return Error::OK;
}

HxTTS::Error HxTTS::sendString(const char* str)
{
    size_t len = strlen(str);
    if (len == 0 || len > 0xffff) {
        return Error::INV_ARG;
    }

    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_BUFFER_CMD_RESET, HX_REQ_TIMEOUT_MS));

    uint16_t data_length = static_cast<uint16_t>(len);
    CHECK_COMM_CALL(hm_comm_reg_write(&transport, HM_REG_BUFFER_LEN_ADDR, &data_length, 2, HX_REQ_TIMEOUT_MS));
    CHECK_COMM_CALL(hm_comm_reg_write_u8(&transport, HM_REG_CMD_ADDR, HM_BUFFER_CMD_ALLOCATE, HX_REQ_TIMEOUT_MS));

    size_t tx_chunk_num = data_length / MAX_PAYLOAD_LEN;
    size_t tx_rem_data  = data_length % MAX_PAYLOAD_LEN;
    for (size_t i = 0; i < tx_chunk_num; i++) {
        const char* chunk = str + i * MAX_PAYLOAD_LEN;
        CHECK_COMM_CALL(
            hm_comm_reg_write(&transport, HM_REG_BUFFER_DATA_ADDR, chunk, MAX_PAYLOAD_LEN, HX_REQ_TIMEOUT_MS));
    }
    if (tx_rem_data != 0) {
        const char* chunk = str + tx_chunk_num * MAX_PAYLOAD_LEN;
        CHECK_COMM_CALL(hm_comm_reg_write(&transport, HM_REG_BUFFER_DATA_ADDR, chunk, tx_rem_data, HX_REQ_TIMEOUT_MS));
    }

    return Error::OK;
}
