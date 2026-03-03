#include <gtest/gtest.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <string>
#include <system_error>

#include "internal/time-tools.h"

namespace {
using namespace LogManager::Internal;

std::tm expectedLocalTime(std::time_t timestamp) {
    std::tm tm{};
#if defined(_WIN32)
    const errno_t error_code = localtime_s(&tm, &timestamp);
    if(error_code != 0) {
        throw std::system_error(static_cast<int>(error_code), std::generic_category(), "localtime_s failed");
    }
#else
    errno = 0;
    if(localtime_r(&timestamp, &tm) == nullptr) {
        const int error_code = errno != 0 ? errno : EINVAL;
        throw std::system_error(error_code, std::generic_category(), "localtime_r failed");
    }
#endif

    return tm;
}
} // namespace

TEST(TimeToolsTests, FormatTmFormatsUsingProvidedPattern) {
    std::tm tm{};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 2;
    tm.tm_mday = 5;
    tm.tm_hour = 6;
    tm.tm_min = 7;
    tm.tm_sec = 8;

    const std::string result = formatTm(tm, "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(result, "2024-03-05 06:07:08");
}

TEST(TimeToolsTests, FormatTmReturnsEmptyStringForEmptyPattern) {
    std::tm tm{};
    tm.tm_year = 2024 - 1900;
    tm.tm_mon = 0;
    tm.tm_mday = 1;

    const std::string result = formatTm(tm, "");
    EXPECT_TRUE(result.empty());
}

TEST(TimeToolsTests, ToLocalTimeMatchesPlatformLocalTimeForKnownTimestamp) {
    const std::time_t raw_timestamp = 1700000000;
    const auto timestamp = std::chrono::system_clock::from_time_t(raw_timestamp);

    const std::tm actual = toLocalTime(timestamp);
    const std::tm expected = expectedLocalTime(raw_timestamp);

    EXPECT_EQ(actual.tm_year, expected.tm_year);
    EXPECT_EQ(actual.tm_mon, expected.tm_mon);
    EXPECT_EQ(actual.tm_mday, expected.tm_mday);
    EXPECT_EQ(actual.tm_hour, expected.tm_hour);
    EXPECT_EQ(actual.tm_min, expected.tm_min);
    EXPECT_EQ(actual.tm_sec, expected.tm_sec);
    EXPECT_EQ(actual.tm_wday, expected.tm_wday);
    EXPECT_EQ(actual.tm_yday, expected.tm_yday);
    EXPECT_EQ(actual.tm_isdst, expected.tm_isdst);
}

TEST(TimeToolsTests, ToLocalTimeDoesNotThrowForCurrentTime) {
    const auto now = std::chrono::system_clock::now();
    EXPECT_NO_THROW({
        const std::tm tm = toLocalTime(now);
        (void)tm;
    });
}
