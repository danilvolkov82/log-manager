#include <gtest/gtest.h>

#include <functional>
#include <stdexcept>
#include <string>

#include "sinks/console-sink/console-sink.h"

namespace {
using LogManager::LogDetails;
using LogManager::LogLevel;
using LogManager::Sinks::ConsoleSink::ConsoleSink;

std::string makeConfig(std::string_view message_format, std::string_view log_level) {
    return std::string("{") +
           "\"message_format\":\"" + std::string(message_format) + "\"," +
           "\"log_level\":\"" + std::string(log_level) + "\"" +
           "}";
}

std::string captureStdout(const std::function<void()> &action) {
    testing::internal::CaptureStdout();
    action();
    return testing::internal::GetCapturedStdout();
}

std::string captureStderr(const std::function<void()> &action) {
    testing::internal::CaptureStderr();
    action();
    return testing::internal::GetCapturedStderr();
}
} // namespace

TEST(ConsoleSinkTests, LogWritesUsingConfiguredMessageFormat) {
    ConsoleSink sink;
    sink.configure(makeConfig("[{level}] {tag}: {message}", "VERBOSE"));

    const std::string output = captureStdout([&sink]() {
        sink.log(LogDetails(LogLevel::INFO, "core", "started"));
    });

    EXPECT_EQ(output, "[INFO] core: started\n");
}

TEST(ConsoleSinkTests, LogThrowsBeforeSuccessfulConfiguration) {
    ConsoleSink sink;

    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "started")), std::runtime_error);
}

TEST(ConsoleSinkTests, LogSkipsEntriesBelowConfiguredMinimumLevel) {
    ConsoleSink sink;
    sink.configure(makeConfig("{level}:{message}", "ERROR"));

    const std::string output = captureStdout([&sink]() {
        sink.log(LogDetails(LogLevel::WARN, "core", "ignore-me"));
        sink.log(LogDetails(LogLevel::ERROR, "core", "keep-me"));
    });

    EXPECT_EQ(output, "ERROR:keep-me\n");
}

TEST(ConsoleSinkTests, ConfigureThrowsWhenCalledAfterSuccessfulConfiguration) {
    ConsoleSink sink;

    ASSERT_NO_THROW(sink.configure(makeConfig("{message}", "VERBOSE")));
    EXPECT_THROW(sink.configure(makeConfig("{message}", "VERBOSE")), std::runtime_error);
}

TEST(ConsoleSinkTests, FailedConfigurationDoesNotMarkSinkAsConfigured) {
    ConsoleSink sink;

    const std::string stderr_output = captureStderr([&sink]() {
        sink.configure(R"({"message_format":123,"log_level":"VERBOSE"})");
    });

    EXPECT_NE(stderr_output.find("[WARN] GeneralSinkConfig: failed to apply configuration:"),
              std::string::npos);
    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "message")), std::runtime_error);

    ASSERT_NO_THROW(sink.configure(makeConfig("{message}", "VERBOSE")));

    const std::string output = captureStdout([&sink]() {
        sink.log(LogDetails(LogLevel::INFO, "core", "message"));
    });

    EXPECT_EQ(output, "message\n");
}
