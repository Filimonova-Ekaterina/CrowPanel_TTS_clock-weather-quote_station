#include "time_formatter.h"
#include <cstring>

void TimeFormatter::set_12h_format(bool enabled) { 
    use_12h_format = enabled; 
    ESP_LOGI("TimeFormatter", "12-hour format: %s", enabled ? "enabled" : "disabled");
}

bool TimeFormatter::get_12h_format() { 
    return use_12h_format; 
}

std::string TimeFormatter::format_time_display(int hours, int minutes, int seconds) {
    char buffer[20];
    if (use_12h_format) {
        int display_hours = hours % 12;
        if (display_hours == 0) display_hours = 12;
        const char* period = (hours < 12) ? "AM" : "PM";
        snprintf(buffer, sizeof(buffer), "%2d:%02d %s", display_hours, minutes, period);
    } else {
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
    }
    return std::string(buffer);
}

std::string TimeFormatter::format_time_for_tts(int hours, int minutes) {
    char buffer[50];
    if (use_12h_format) {
        int display_hours = hours % 12;
        if (display_hours == 0) display_hours = 12;
        const char* period = (hours < 12) ? "A M" : "P M";
        snprintf(buffer, sizeof(buffer), "%d %d %s", display_hours, minutes, period);
    } else {
        snprintf(buffer, sizeof(buffer), "%d hours %d minutes", hours, minutes);
    }
    return std::string(buffer);
}

const char* TimeFormatter::format_time_display_c(int hours, int minutes, int seconds) {
    static char buffer[20];
    std::string result = format_time_display(hours, minutes, seconds);
    strncpy(buffer, result.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}

const char* TimeFormatter::format_time_for_tts_c(int hours, int minutes) {
    static char buffer[50];
    std::string result = format_time_for_tts(hours, minutes);
    strncpy(buffer, result.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    return buffer;
}