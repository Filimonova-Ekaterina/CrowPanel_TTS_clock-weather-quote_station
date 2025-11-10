#pragma once
#include <string>
#include "esp_log.h"

class TimeFormatter {
private:
    bool use_12h_format = false;
    
public:
    void set_12h_format(bool enabled);
    bool get_12h_format();
    std::string format_time_display(int hours, int minutes, int seconds);
    std::string format_time_for_tts(int hours, int minutes);
    
    const char* format_time_display_c(int hours, int minutes, int seconds);
    const char* format_time_for_tts_c(int hours, int minutes);
};