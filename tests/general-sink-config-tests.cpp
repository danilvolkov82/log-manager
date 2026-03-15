#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

#include "sinks/general-sink-config.h"

namespace {
using namespace LogManager;
using LogManager::Sinks::GeneralSinkConfig;

constexpr const char *kDefaultMessageFormat = "[{timestamp}] [{level} {tag}] {message} {exception}";

bool captureConfigureResultAndStderr(
    GeneralSinkConfig &config,
    const std::string &json_config,
    std::string &stderr_output) {
    testing::internal::CaptureStderr();
    const bool result = config.configure(json_config);
    stderr_output = testing::internal::GetCapturedStderr();
    return result;
}

class DerivedGeneralSinkConfig : public GeneralSinkConfig {
public:
    int validate_calls{0};
    int process_calls{0};
    bool validate_should_throw{false};
    bool should_throw{false};
    std::string last_json_config;

protected:
    void validate(const std::string &json_config) const override {
        DerivedGeneralSinkConfig &self = const_cast<DerivedGeneralSinkConfig &>(*this);
        ++self.validate_calls;
        self.last_json_config = json_config;

        if(validate_should_throw) {
            throw std::runtime_error("derived validate failure");
        }

        GeneralSinkConfig::validate(json_config);
    }

    void processConfig(const std::string &json_config) override {
        ++process_calls;
        last_json_config = json_config;

        if(should_throw) {
            throw std::runtime_error("derived config failure");
        }

        GeneralSinkConfig::processConfig(json_config);
    }
};
} // namespace

TEST(GeneralSinkConfigTests, DefaultsAreReturnedBeforeConfiguration) {
    GeneralSinkConfig config;

    EXPECT_EQ(config.getMinLevel(), LogLevel::INFO);
    EXPECT_EQ(config.getMessageFormat(), kDefaultMessageFormat);
}

TEST(GeneralSinkConfigTests, ConfigureAppliesNumericAndNamedLogLevels) {
    GeneralSinkConfig config;

    EXPECT_TRUE(config.configure(R"({"message_format":"[{level}] {message}","log_level":4})"));
    EXPECT_EQ(config.getMinLevel(), LogLevel::FATAL);
    EXPECT_EQ(config.getMessageFormat(), "[{level}] {message}");

    EXPECT_TRUE(config.configure(R"({"log_level":"warn"})"));
    EXPECT_EQ(config.getMinLevel(), LogLevel::WARN);
    EXPECT_EQ(config.getMessageFormat(), kDefaultMessageFormat);
}

TEST(GeneralSinkConfigTests, ConfigureWarnsAndKeepsPreviousStateForInvalidBaseFieldType) {
    GeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"ERROR"})"));

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":123,"log_level":"WARN"})",
        stderr_output
    );

    EXPECT_FALSE(result);
    EXPECT_NE(stderr_output.find("[WARN] GeneralSinkConfig: failed to apply configuration:"),
              std::string::npos);
    EXPECT_EQ(config.getMinLevel(), LogLevel::ERROR);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}

TEST(GeneralSinkConfigTests, ConfigureWarnsAndKeepsPreviousStateForInvalidDerivedFieldType) {
    GeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"INFO"})"));

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":"changed","log_level":"TRACE"})",
        stderr_output
    );

    EXPECT_FALSE(result);
    EXPECT_NE(stderr_output.find("log_level name is not supported"), std::string::npos);
    EXPECT_EQ(config.getMinLevel(), LogLevel::INFO);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}

TEST(GeneralSinkConfigTests, ConfigureWarnsAndKeepsPreviousStateForOutOfRangeNumericLogLevel) {
    GeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"INFO"})"));

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"log_level":999})",
        stderr_output
    );

    EXPECT_FALSE(result);
    EXPECT_NE(stderr_output.find("log_level is out of supported range"), std::string::npos);
    EXPECT_EQ(config.getMinLevel(), LogLevel::INFO);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}

TEST(GeneralSinkConfigTests, ConfigureWarnsForMalformedOrWrongShapePayloads) {
    GeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"WARN"})"));

    std::string malformed_stderr;
    const bool malformed_result = captureConfigureResultAndStderr(config, "{bad json}", malformed_stderr);
    EXPECT_FALSE(malformed_result);
    EXPECT_NE(malformed_stderr.find("[WARN] GeneralSinkConfig: failed to apply configuration:"),
              std::string::npos);
    EXPECT_EQ(config.getMinLevel(), LogLevel::WARN);
    EXPECT_EQ(config.getMessageFormat(), "kept");

    std::string shape_stderr;
    const bool shape_result = captureConfigureResultAndStderr(config, "[]", shape_stderr);
    EXPECT_FALSE(shape_result);
    EXPECT_NE(shape_stderr.find("config must be a JSON object"), std::string::npos);
    EXPECT_EQ(config.getMinLevel(), LogLevel::WARN);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}

TEST(GeneralSinkConfigTests, ConfigureDelegatesToProcessConfigOverride) {
    DerivedGeneralSinkConfig config;

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":"[{tag}] {message}","log_level":"fatal"})",
        stderr_output
    );

    EXPECT_TRUE(result);
    EXPECT_TRUE(stderr_output.empty());
    EXPECT_EQ(config.validate_calls, 1);
    EXPECT_EQ(config.process_calls, 1);
    EXPECT_EQ(config.last_json_config, R"({"message_format":"[{tag}] {message}","log_level":"fatal"})");
    EXPECT_EQ(config.getMinLevel(), LogLevel::FATAL);
    EXPECT_EQ(config.getMessageFormat(), "[{tag}] {message}");
}

TEST(GeneralSinkConfigTests, ConfigureRunsValidateBeforeProcessConfig) {
    DerivedGeneralSinkConfig config;

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":"[{tag}] {message}","log_level":"INFO"})",
        stderr_output
    );

    EXPECT_TRUE(result);
    EXPECT_TRUE(stderr_output.empty());
    EXPECT_EQ(config.validate_calls, 1);
    EXPECT_EQ(config.process_calls, 1);
}

TEST(GeneralSinkConfigTests, ConfigureWarnsAndSkipsProcessConfigWhenValidateThrows) {
    DerivedGeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"INFO"})"));
    config.validate_should_throw = true;

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":"changed","log_level":"WARN"})",
        stderr_output
    );

    EXPECT_FALSE(result);
    EXPECT_NE(stderr_output.find("derived validate failure"), std::string::npos);
    EXPECT_EQ(config.validate_calls, 2);
    EXPECT_EQ(config.process_calls, 1);
    EXPECT_EQ(config.getMinLevel(), LogLevel::INFO);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}

TEST(GeneralSinkConfigTests, ConfigureWarnsAndKeepsStateWhenOverrideThrows) {
    DerivedGeneralSinkConfig config;
    ASSERT_TRUE(config.configure(R"({"message_format":"kept","log_level":"INFO"})"));
    config.should_throw = true;

    std::string stderr_output;
    const bool result = captureConfigureResultAndStderr(
        config,
        R"({"message_format":"changed","log_level":"WARN"})",
        stderr_output
    );

    EXPECT_FALSE(result);
    EXPECT_NE(stderr_output.find("derived config failure"), std::string::npos);
    EXPECT_EQ(config.validate_calls, 2);
    EXPECT_EQ(config.process_calls, 2);
    EXPECT_EQ(config.getMinLevel(), LogLevel::INFO);
    EXPECT_EQ(config.getMessageFormat(), "kept");
}
