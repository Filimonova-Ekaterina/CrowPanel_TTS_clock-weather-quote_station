#include "tts_bridge.h"
#include "HxTTS.h"
#include "esp_log.h"

static const char *TAG = "tts_bridge";

extern HxTTS *g_hx_tts;

extern "C" void start_tts_playback_c(const char *text)
{
    if (!g_hx_tts || !text) {
        ESP_LOGW(TAG, "TTS backend not ready or null text");
        return;
    }

    auto err = g_hx_tts->sendString(text);
    if (err != HxTTS::Error::OK) {
        ESP_LOGE(TAG, "sendString failed");
        return;
    }

    err = g_hx_tts->startPlayback();
    if (err != HxTTS::Error::OK) {
        ESP_LOGE(TAG, "startPlayback failed");
        return;
    }
}
