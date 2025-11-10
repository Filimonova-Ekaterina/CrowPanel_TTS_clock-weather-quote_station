#include "tts_bridge.h"

static start_tts_cb_t g_cb = 0;

void start_tts_playback_c(const char *text)
{
    if (g_cb) {
        g_cb(text);
    }
}

void register_start_tts_cb(start_tts_cb_t cb)
{
    g_cb = cb;
}
