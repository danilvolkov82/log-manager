#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

#include "sinks/file-sink/file-sink.h"

namespace {
using LogManager::LogDetails;
using LogManager::LogLevel;
using LogManager::Sinks::FileSink::FileSink;
namespace fs = std::filesystem;

class ScopedTempDir {
public:
    ScopedTempDir() {
        static std::atomic_uint64_t counter{0};
        const std::uint64_t id = counter.fetch_add(1, std::memory_order_relaxed);
        _path = fs::temp_directory_path() / ("log-manager-file-sink-tests-" + std::to_string(id));
        std::error_code ec;
        fs::create_directories(_path, ec);
    }

    ~ScopedTempDir() {
        std::error_code ec;
        fs::remove_all(_path, ec);
    }

    const fs::path &path() const {
        return _path;
    }

private:
    fs::path _path;
};

std::string jsonEscape(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size());

    for(const char c : value) {
        switch(c) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += c;
            break;
        }
    }

    return escaped;
}

void writeText(const fs::path &path, const std::string &content) {
    std::ofstream output(path, std::ios::out | std::ios::trunc);
    output << content;
}

std::string readText(const fs::path &path) {
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string makeConfig(
    const fs::path &file_path,
    std::string_view message_format,
    std::string_view log_level,
    std::string_view rotation = "NONE",
    std::uint64_t max_file_size = 0,
    std::int32_t max_backup_files = -1,
    std::uint32_t max_file_age_days = 0) {
    return std::string("{") +
           "\"filename_template\":\"" + jsonEscape(file_path.string()) + "\"," +
           "\"message_format\":\"" + jsonEscape(std::string(message_format)) + "\"," +
           "\"log_level\":\"" + jsonEscape(std::string(log_level)) + "\"," +
           "\"rotation\":\"" + jsonEscape(std::string(rotation)) + "\"," +
           "\"max_file_size\":" + std::to_string(max_file_size) + "," +
           "\"max_backup_files\":" + std::to_string(max_backup_files) + "," +
           "\"max_file_age_days\":" + std::to_string(max_file_age_days) +
           "}";
}

std::string captureConfigureStderr(FileSink &sink, const std::string &json_config) {
    testing::internal::CaptureStderr();
    sink.configure(json_config);
    return testing::internal::GetCapturedStderr();
}
} // namespace

TEST(FileSinkTests, LogWritesUsingConfiguredFilenameAndMessageFormat) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "app.log";
    FileSink sink;

    sink.configure(makeConfig(file, "[{level}] {tag}: {message}", "VERBOSE"));
    sink.log(LogDetails(LogLevel::INFO, "core", "started"));

    ASSERT_TRUE(fs::exists(file));
    EXPECT_EQ(readText(file), "[INFO] core: started\n");
}

TEST(FileSinkTests, LogThrowsBeforeSuccessfulConfiguration) {
    FileSink sink;

    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "started")), std::runtime_error);
}

TEST(FileSinkTests, LogSkipsEntriesBelowConfiguredMinimumLevel) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "filtered.log";
    FileSink sink;

    sink.configure(makeConfig(file, "{level}:{message}", "ERROR"));
    sink.log(LogDetails(LogLevel::WARN, "core", "ignore-me"));
    sink.log(LogDetails(LogLevel::ERROR, "core", "keep-me"));

    ASSERT_TRUE(fs::exists(file));
    EXPECT_EQ(readText(file), "ERROR:keep-me\n");
}

TEST(FileSinkTests, ConfigureThrowsWhenCalledAfterSuccessfulConfiguration) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "app.log";
    FileSink sink;

    ASSERT_NO_THROW(sink.configure(makeConfig(file, "{message}", "VERBOSE")));
    EXPECT_THROW(sink.configure(makeConfig(file, "{message}", "VERBOSE")), std::runtime_error);
}

TEST(FileSinkTests, FailedConfigurationDoesNotMarkSinkAsConfigured) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "app.log";
    FileSink sink;

    const std::string stderr_output = captureConfigureStderr(
        sink,
        std::string("{") +
        "\"filename_template\":\"" + jsonEscape(file.string()) + "\"," +
        "\"message_format\":\"{message}\"," +
        "\"log_level\":\"VERBOSE\"," +
        "\"rotation\":1," +
        "\"max_file_size\":0," +
        "\"max_backup_files\":1," +
        "\"max_file_age_days\":0" +
        "}"
    );

    EXPECT_NE(stderr_output.find("rotation must be a string"), std::string::npos);
    EXPECT_THROW(sink.log(LogDetails(LogLevel::INFO, "core", "second")), std::runtime_error);

    ASSERT_NO_THROW(sink.configure(makeConfig(file, "{message}", "VERBOSE", "STARTUP", 0, 1)));
    sink.log(LogDetails(LogLevel::INFO, "core", "second"));
    EXPECT_EQ(readText(file), "second\n");
}
