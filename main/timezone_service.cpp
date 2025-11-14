#include "timezone_service.h"
#include "location_service.h"
#include "esp_log.h"
#include <string>
#include <cstring>

static const char *TAG = "Timezone";

std::string TimezoneService::detect_timezone() {
    ESP_LOGI(TAG, "Starting timezone detection...");
    
    const char* detected_tz = get_timezone_by_location();
    
    if (detected_tz && strcmp(detected_tz, "GMT0") != 0 && strcmp(detected_tz, "Manual") != 0) {
        ESP_LOGI(TAG, "Detected timezone from location: '%s'", detected_tz);
        std::string posix_tz = convert_iana_to_posix(detected_tz);
        ESP_LOGI(TAG, "Converted to POSIX: '%s'", posix_tz.c_str());
        return posix_tz;
    }
    
    ESP_LOGW(TAG, "Could not detect timezone, using fallback GMT0");
    return "GMT0";
}

std::string TimezoneService::convert_iana_to_posix(const std::string& iana_tz) {
    ESP_LOGI(TAG, "Converting IANA: %s", iana_tz.c_str());
    
    if (iana_tz.empty()) {
        ESP_LOGW(TAG, "Empty IANA timezone, using GMT0");
        return "GMT0";
    }
    
    if (iana_tz == "Europe/Moscow") return "MSK-3";
    else if (iana_tz == "Europe/Kaliningrad") return "EET-2";
    else if (iana_tz == "Europe/Samara") return "SAMT-4";
    else if (iana_tz == "Europe/Yekaterinburg") return "YEKT-5";
    else if (iana_tz == "Europe/Omsk") return "OMST-6";
    else if (iana_tz == "Europe/Krasnoyarsk") return "KRAT-7";
    else if (iana_tz == "Europe/Irkutsk") return "IRKT-8";
    else if (iana_tz == "Asia/Yakutsk") return "YAKT-9";
    else if (iana_tz == "Asia/Vladivostok") return "VLAT-10";
    else if (iana_tz == "Asia/Magadan") return "MAGT-11";
    else if (iana_tz == "Asia/Kamchatka") return "PETT-12";
    else if (iana_tz == "Europe/Volgograd") return "MSK-3";
    else if (iana_tz == "Europe/Astrakhan") return "SAMT-4";
    else if (iana_tz == "Europe/Ulyanovsk") return "SAMT-4";
    else if (iana_tz == "Asia/Novosibirsk") return "NOVT-6";
    else if (iana_tz == "Asia/Novokuznetsk") return "KRAT-7";
    
    else if (iana_tz == "Europe/Kiev") return "EET-2EEST";
    else if (iana_tz == "Europe/Minsk") return "MSK-3";
    else if (iana_tz == "Asia/Almaty") return "ALMT-6";
    else if (iana_tz == "Asia/Qyzylorda") return "QYZT-5";
    else if (iana_tz == "Asia/Aqtobe") return "AQTT-5";
    else if (iana_tz == "Asia/Aqtau") return "AQTT-5";
    else if (iana_tz == "Asia/Atyrau") return "AQTT-5";
    else if (iana_tz == "Asia/Oral") return "ORAT-5";
    
    else if (iana_tz == "Europe/London") return "GMT0BST";
    else if (iana_tz == "Europe/Paris" || iana_tz == "Europe/Berlin" || 
             iana_tz == "Europe/Rome" || iana_tz == "Europe/Madrid" ||
             iana_tz == "Europe/Amsterdam" || iana_tz == "Europe/Brussels" ||
             iana_tz == "Europe/Vienna" || iana_tz == "Europe/Prague" ||
             iana_tz == "Europe/Warsaw" || iana_tz == "Europe/Budapest") {
        return "CET-1CEST";
    }
    else if (iana_tz == "Europe/Athens" || iana_tz == "Europe/Helsinki" ||
             iana_tz == "Europe/Sofia" || iana_tz == "Europe/Tallinn" ||
             iana_tz == "Europe/Riga" || iana_tz == "Europe/Vilnius" ||
             iana_tz == "Europe/Bucharest" || iana_tz == "Europe/Istanbul") {
        return "EET-2EEST";
    }
    
    else if (iana_tz == "America/New_York" || iana_tz == "America/Toronto") return "EST5EDT";
    else if (iana_tz == "America/Chicago" || iana_tz == "America/Winnipeg") return "CST6CDT";
    else if (iana_tz == "America/Denver" || iana_tz == "America/Edmonton") return "MST7MDT";
    else if (iana_tz == "America/Los_Angeles" || iana_tz == "America/Vancouver") return "PST8PDT";
    
    else if (iana_tz == "Asia/Tokyo") return "JST-9";
    else if (iana_tz == "Asia/Shanghai" || iana_tz == "Asia/Beijing") return "CST-8";
    else if (iana_tz == "Asia/Seoul") return "KST-9";
    else if (iana_tz == "Asia/Singapore") return "SGT-8";
    else if (iana_tz == "Asia/Dubai") return "GST-4";
    else if (iana_tz == "Asia/Bangkok") return "ICT-7";
    else if (iana_tz == "Asia/Jakarta") return "WIB-7";
    
    else if (iana_tz == "Australia/Sydney") return "AEST-10AEDT";
    else if (iana_tz == "Australia/Melbourne") return "AEST-10AEDT";
    else if (iana_tz == "Australia/Perth") return "AWST-8";
    
    else if (iana_tz.find("Europe") != std::string::npos) return "CET-1CEST";
    else if (iana_tz.find("Asia") != std::string::npos) return "MSK-3";
    else if (iana_tz.find("America") != std::string::npos) return "EST5EDT";
    
    ESP_LOGW(TAG, "Unknown IANA timezone: %s, using GMT0", iana_tz.c_str());
    return "GMT0";
}