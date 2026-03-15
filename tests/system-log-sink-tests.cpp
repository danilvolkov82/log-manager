#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "internal/sinks/system-log-sink-tools.h"
#include "sinks/system-log-sink/system-log-sink.h"

namespace {
using LogManager::LogDetails;
using LogManager::LogLevel;
using LogManager::Internal::Sinks::setSystemLogWriteFnForTests;
using LogManager::Internal::Sinks::toSystemLogPriority;
using LogManager::Sinks::SystemLogSink::SystemLogSink;

int captured_priority = 0;
std::string captured_message;
int write_call_count = 0;

std::string makeConfig(std::string_view message_format, std::string_view log_level) {
    return std::string("{") +
           "\"message_format\":\"" + std::string(message_format) + "\"," +
           "\"log_level\":\"" + std::string(log_level) + "\"" +
           "}";
}

std::string captureStderr(const std::function<void()> &action) {
    testing::internal::CaptureStderr();
    action();
    return testing::internal::GetCapturedStderr();
}

void captureSystemLogWrite(int priority, std::string_view message) {
    captured_priority = priority;
    captured_message = std::string(message);
    ++write_call_count;
}

class ScopedSystemLogWriterOverride {
public:
    ScopedSystemLogWriterOverride() {
        captured_priority = 0;
        captured_message.clear();
        write_call_count = 0;
        setSystemLogWriteFnForTests(captureSystemLogWrite);
    }

    ~ScopedSystemLogWriterOverride() {
        setSystemLogWriteFnForTests(nullptr);
    }
};
} // namespace

TEST(SystemLogSinkTests, LogWritesUsingConfiguredMessageFormatAndPriority) {
    ScopedSystemLogWriterOverride writer_override;
    SystemLogSink sink;
    sink.configure(makeConfig("[{level}] {tag}: {message}", "VERBOSE"));

    ASSERT_TRUE(sink.isConfigured());
    sink.log(LogDetails(LogLevel::INFO, "core", "started"));

    EXPECT_EQ(write_call_count, 1);
    EXPECT_EQ(captured_priority, toSystemLogPriority(LogLevel::INFO));
    EXPECT_EQ(captured_message, "[INFO] core: started");
}

TEST(SystemLogSinkTests, IsConfiguredReturnsFalseBeforeSuccessfulConfiguration) {
    SystemLogSink sink;

    EXPECT_FALSE(sink.isConfigured());
}

TEST(SystemLogSinkTests, LogThrowsBeforeSuccessfulConfiguration) {
    ScopedSystemLogWriterOverride writer_override;
    SystemLogSink sink;

    EXPECT_FALSE(sink.isConfigured());
    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "started")), std::runtime_error);
    EXPECT_EQ(write_call_count, 0);
}

TEST(SystemLogSinkTests, LogSkipsEntriesBelowConfiguredMinimumLevel) {
    ScopedSystemLogWriterOverride writer_override;
    SystemLogSink sink;
    sink.configure(makeConfig("{level}:{message}", "ERROR"));

    sink.log(LogDetails(LogLevel::WARN, "core", "ignore-me"));
    sink.log(LogDetails(LogLevel::ERROR, "core", "keep-me"));

    EXPECT_EQ(write_call_count, 1);
    EXPECT_EQ(captured_priority, toSystemLogPriority(LogLevel::ERROR));
    EXPECT_EQ(captured_message, "ERROR:keep-me");
}

TEST(SystemLogSinkTests, ConfigureThrowsWhenCalledAfterSuccessfulConfiguration) {
    SystemLogSink sink;

    ASSERT_NO_THROW(sink.configure(makeConfig("{message}", "VERBOSE")));
    ASSERT_TRUE(sink.isConfigured());
    EXPECT_THROW(sink.configure(makeConfig("{message}", "VERBOSE")), std::runtime_error);
}

TEST(SystemLogSinkTests, FailedConfigurationDoesNotMarkSinkAsConfigured) {
    ScopedSystemLogWriterOverride writer_override;
    SystemLogSink sink;

    const std::string stderr_output = captureStderr([&sink]() {
        sink.configure(R"({"message_format":123,"log_level":"VERBOSE"})");
    });

    EXPECT_NE(stderr_output.find("[WARN] GeneralSinkConfig: failed to apply configuration:"),
              std::string::npos);
    EXPECT_FALSE(sink.isConfigured());
    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "message")), std::runtime_error);
    EXPECT_EQ(write_call_count, 0);

    ASSERT_NO_THROW(sink.configure(makeConfig("{message}", "VERBOSE")));
    ASSERT_TRUE(sink.isConfigured());
    sink.log(LogDetails(LogLevel::INFO, "core", "message"));

    EXPECT_EQ(write_call_count, 1);
    EXPECT_EQ(captured_priority, toSystemLogPriority(LogLevel::INFO));
    EXPECT_EQ(captured_message, "message");
}
