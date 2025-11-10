#ifndef TIMEZONE_SERVICE_H
#define TIMEZONE_SERVICE_H

#include <string>

class TimezoneService {
public:
    static std::string detect_timezone();
    static std::string convert_iana_to_posix(const std::string& iana_tz);
};

#endif