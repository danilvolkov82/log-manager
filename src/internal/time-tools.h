#ifndef LOG_MANAGER_INTERNAL_TIME_TOOLS
#define LOG_MANAGER_INTERNAL_TIME_TOOLS
#pragma once

#include <ctime>
#include <chrono>
#include <string>

/**
 * @file time-tools.h
 * @brief Internal helpers for converting and formatting local time values.
 */
namespace LogManager::Internal {
/**
 * @brief Converts a system clock timestamp to local calendar time.
 *
 * Uses platform-specific thread-safe APIs:
 * - `localtime_s` on Windows
 * - `localtime_r` on POSIX systems
 *
 * @param timestamp Input timestamp in system clock domain.
 * @return Converted local time as `std::tm`.
 * @throws std::system_error If conversion fails on the current platform.
 */
std::tm toLocalTime(const std::chrono::system_clock::time_point &timestamp);

/**
 * @brief Formats a `std::tm` with a `std::put_time`-compatible pattern.
 *
 * @param tm Local time structure to format.
 * @param pattern `strftime`-style pattern string (for example `%Y-%m-%d`).
 * @return Formatted text. Returns an empty string when the pattern expands to
 *         an empty output.
 */
std::string formatTm(const std::tm &tm, const std::string &pattern);
}
#endif // LOG_MANAGER_INTERNAL_TIME_TOOLS
