/**
 * @file time-tools.cpp
 * @brief Implementation of internal local-time helpers.
 */
#include "time-tools.h"
#include <cerrno>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <system_error>

namespace LogManager::Internal {
std::tm
toLocalTime(const std::chrono::system_clock::time_point &timestamp) {
    const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm{};
#ifdef _WIN32
    const errno_t error_code = localtime_s(&tm, &time);
    if(error_code != 0) {
        throw std::system_error(static_cast<int>(error_code), std::generic_category(), "localtime_s failed");
    }
#else
    errno = 0;
    if(localtime_r(&time, &tm) == nullptr) {
        const int error_code = errno != 0 ? errno : EINVAL;
        throw std::system_error(error_code, std::generic_category(), "localtime_r failed");
    }
#endif

    return tm;
};

std::string
formatTm(const std::tm &tm, const std::string &pattern) {
    std::ostringstream stream;
    stream << std::put_time(&tm, pattern.c_str());

    return stream.str();
};
}
