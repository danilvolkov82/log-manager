#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "internal/log-message-tools.h"
#include "internal/time-tools.h"

namespace {
using namespace LogManager;
using namespace LogManager::Internal;

LogDetails makeEntry() {
    LogDetails entry(LogLevel::WARN, "net", "connection-lost");
    entry.timestamp = std::chrono::system_clock::from_time_t(1700000000);
    entry.thread_id = std::this_thread::get_id();
    entry.exception = std::make_exception_ptr(std::runtime_error("boom"));
    return entry;
}

std::string expectedTimestamp(const LogDetails &entry) {
    try {
        const std::tm tm = toLocalTime(entry.timestamp);
        return formatTm(tm, "%Y-%m-%d %H:%M:%S");
    } catch(...) {
        return "0000-00-00 00:00:00";
    }
}

std::string expectedDate(const LogDetails &entry) {
    try {
        const std::tm tm = toLocalTime(entry.timestamp);
        return formatTm(tm, "%Y-%m-%d");
    } catch(...) {
        return "0000-00-00";
    }
}

std::string expectedDateTime(const LogDetails &entry) {
    try {
        const std::tm tm = toLocalTime(entry.timestamp);
        return formatTm(tm, "%Y%m%d-%H%M%S");
    } catch(...) {
        return "00000000-000000";
    }
}
} // namespace

TEST(LogMessageToolsTests, ThreadIdToStringMatchesStreamSerialization) {
    const std::thread::id thread_id = std::this_thread::get_id();
    std::ostringstream stream;
    stream << thread_id;
    EXPECT_EQ(threadIdToString(thread_id), stream.str());
}

TEST(LogMessageToolsTests, ExceptionToStringReturnsEmptyForNullExceptionPtr) {
    const std::exception_ptr exception;
    EXPECT_TRUE(exceptionToString(exception).empty());
}

TEST(LogMessageToolsTests, ExceptionToStringReturnsMessageForStdException) {
    const std::exception_ptr exception = std::make_exception_ptr(std::runtime_error("broken"));
    EXPECT_EQ(exceptionToString(exception), "broken");
}

TEST(LogMessageToolsTests, ExceptionToStringReturnsFallbackForUnknownExceptionType) {
    const std::exception_ptr exception = std::make_exception_ptr(42);
    EXPECT_EQ(exceptionToString(exception), "unknown exception");
}

TEST(LogMessageToolsTests, LevelToStringMapsKnownAndUnknownValues) {
    EXPECT_EQ(levelToString(LogLevel::VERBOSE), "VERBOSE");
    EXPECT_EQ(levelToString(LogLevel::INFO), "INFO");
    EXPECT_EQ(levelToString(LogLevel::WARN), "WARN");
    EXPECT_EQ(levelToString(LogLevel::ERROR), "ERROR");
    EXPECT_EQ(levelToString(LogLevel::FATAL), "FATAL");
    EXPECT_EQ(levelToString(static_cast<LogLevel>(255)), "UNKNOWN");
}

TEST(LogMessageToolsTests, RenderMessageTemplateReplacesKnownPlaceholdersAndKeepsUnknown) {
    const LogDetails entry = makeEntry();

    const std::string rendered = renderMessageTemplate(
        "[{timestamp}] [{level}] [{tag}] {message} {exception} {thread_id} {unknown}",
        entry);

    const std::string expected =
        "[" + expectedTimestamp(entry) + "] [WARN] [net] connection-lost boom " +
        threadIdToString(entry.thread_id) + " {unknown}";
    EXPECT_EQ(rendered, expected);
}

TEST(LogMessageToolsTests, RenderFilenameTemplateReplacesKnownPlaceholdersAndKeepsUnknown) {
    const LogDetails entry = makeEntry();

    const std::string rendered = renderFilenameTemplate(
        "app-{date}-{datetime}-{level}-{tag}-{unknown}.log",
        entry);

    const std::string expected = "app-" + expectedDate(entry) + "-" +
                                 expectedDateTime(entry) + "-WARN-net-{unknown}.log";
    EXPECT_EQ(rendered, expected);
}
