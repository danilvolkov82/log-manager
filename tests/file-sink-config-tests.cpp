#include <gtest/gtest.h>

#include <string>

#include "sinks/file-sink/file-sink-config.h"

namespace {
using namespace LogManager::Sinks::FileSink;

std::string captureConfigureStderr(FileSinkConfig &config, const std::string &json_config) {
    testing::internal::CaptureStderr();
    config.configure(json_config);
    return testing::internal::GetCapturedStderr();
}
} // namespace

TEST(FileSinkConfigTests, ConfigureParsesRotationNamesSeparatedByPipe) {
    FileSinkConfig config;

    config.configure(R"({"rotation":"DAILY|SIZE|STARTUP"})");

    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::DAILY));
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::STARTUP));
}

TEST(FileSinkConfigTests, ConfigureParsesRotationNamesCaseInsensitivelyAndWithWhitespace) {
    FileSinkConfig config;

    config.configure(R"({"rotation":" daily | startup "})");

    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::DAILY));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::STARTUP));
}

TEST(FileSinkConfigTests, ConfigureWarnsAndKeepsPreviousStateForNumericRotation) {
    FileSinkConfig config;
    config.configure(R"({"rotation":"SIZE"})");

    const std::string stderr_output = captureConfigureStderr(
        config,
        R"({"rotation":3})"
    );

    EXPECT_NE(stderr_output.find("rotation must be a string"), std::string::npos);
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::DAILY));
}

TEST(FileSinkConfigTests, ConfigureWarnsAndKeepsPreviousStateForUnknownRotationToken) {
    FileSinkConfig config;
    config.configure(R"({"rotation":"DAILY"})");

    const std::string stderr_output = captureConfigureStderr(
        config,
        R"({"rotation":"DAILY|WEEKLY"})"
    );

    EXPECT_NE(stderr_output.find("rotation token is not supported"), std::string::npos);
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::DAILY));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::STARTUP));
}

TEST(FileSinkConfigTests, ConfigureKeepsDerivedStateWhenRotationValidationFails) {
    FileSinkConfig config;
    config.configure(R"({"rotation":"STARTUP","max_file_size":10})");

    const std::string stderr_output = captureConfigureStderr(
        config,
        R"({"rotation":"SIZE|","max_file_size":55})"
    );

    EXPECT_NE(stderr_output.find("rotation token is empty"), std::string::npos);
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::STARTUP));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_EQ(config.getMaxFileSize(), 10u);
}

TEST(FileSinkConfigTests, ConfigureKeepsBaseAndDerivedStateWhenBaseParsingFails) {
    FileSinkConfig config;
    config.configure(R"({"rotation":"DAILY","log_level":"INFO"})");

    const std::string stderr_output = captureConfigureStderr(
        config,
        R"({"rotation":"SIZE","log_level":"TRACE"})"
    );

    EXPECT_NE(stderr_output.find("log_level name is not supported"), std::string::npos);
    EXPECT_TRUE(hasFlag(config.getRotation(), RotationMode::DAILY));
    EXPECT_FALSE(hasFlag(config.getRotation(), RotationMode::SIZE));
    EXPECT_EQ(config.getMinLevel(), LogManager::LogLevel::INFO);
}
