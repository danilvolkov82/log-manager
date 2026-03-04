#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "internal/sinks/file-sink-tools.h"

namespace {
using namespace LogManager::Internal::Sinks;
namespace fs = std::filesystem;

class ScopedTempDir {
public:
    ScopedTempDir() {
        static std::atomic_uint64_t counter{0};
        const std::uint64_t id = counter.fetch_add(1, std::memory_order_relaxed);
        _path = fs::temp_directory_path() / ("log-manager-file-sink-tools-tests-" + std::to_string(id));
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

bool setAgeHours(const fs::path &path, int hours) {
    std::error_code ec;
    fs::last_write_time(path, fs::file_time_type::clock::now() - std::chrono::hours(hours), ec);
    return !ec;
}
} // namespace

TEST(FileSinkToolsTests, AppendLineAddsTrailingNewlineAndAppends) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "app.log";

    appendLine(file, "first");
    appendLine(file, "second");

    EXPECT_EQ(readText(file), "first\nsecond\n");
}

TEST(FileSinkToolsTests, EnsureParentDirectoryCreatesNestedDirectories) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "nested" / "logs" / "app.log";

    ensureParentDirectory(file);

    EXPECT_TRUE(fs::exists(dir.path() / "nested" / "logs"));
    EXPECT_TRUE(fs::is_directory(dir.path() / "nested" / "logs"));
}

TEST(FileSinkToolsTests, ShouldRotateBySizeUsesCurrentAndIncomingBytes) {
    ScopedTempDir dir;
    const fs::path file = dir.path() / "size.log";
    writeText(file, "12345");

    EXPECT_FALSE(shouldRotateBySize(file, 1, 0));
    EXPECT_TRUE(shouldRotateBySize(file, 1, 5));
    EXPECT_FALSE(shouldRotateBySize(file, 2, 7));
    EXPECT_TRUE(shouldRotateBySize(file, 3, 7));
    EXPECT_FALSE(shouldRotateBySize(dir.path() / "missing.log", 10, 20));
}

TEST(FileSinkToolsTests, RotateFileShiftsBackupsAndPrunesToMaxCount) {
    ScopedTempDir dir;
    const fs::path current = dir.path() / "app.log";

    writeText(current, "L1");
    rotateFile(current, 2);
    EXPECT_FALSE(fs::exists(current));
    ASSERT_TRUE(fs::exists(dir.path() / "app.log.1"));
    EXPECT_EQ(readText(dir.path() / "app.log.1"), "L1");

    writeText(current, "L2");
    rotateFile(current, 2);
    ASSERT_TRUE(fs::exists(dir.path() / "app.log.1"));
    ASSERT_TRUE(fs::exists(dir.path() / "app.log.2"));
    EXPECT_EQ(readText(dir.path() / "app.log.1"), "L2");
    EXPECT_EQ(readText(dir.path() / "app.log.2"), "L1");

    writeText(current, "L3");
    rotateFile(current, 2);
    ASSERT_TRUE(fs::exists(dir.path() / "app.log.1"));
    ASSERT_TRUE(fs::exists(dir.path() / "app.log.2"));
    EXPECT_FALSE(fs::exists(dir.path() / "app.log.3"));
    EXPECT_EQ(readText(dir.path() / "app.log.1"), "L3");
    EXPECT_EQ(readText(dir.path() / "app.log.2"), "L2");
}

TEST(FileSinkToolsTests, RotateFileWithZeroBackupsRemovesCurrentAndExistingBackups) {
    ScopedTempDir dir;
    const fs::path current = dir.path() / "app.log";
    const fs::path backup1 = dir.path() / "app.log.1";
    const fs::path backup2 = dir.path() / "app.log.2";

    writeText(current, "L0");
    writeText(backup1, "L-1");
    writeText(backup2, "L-2");

    rotateFile(current, 0);

    EXPECT_FALSE(fs::exists(current));
    EXPECT_FALSE(fs::exists(backup1));
    EXPECT_FALSE(fs::exists(backup2));
}

TEST(FileSinkToolsTests, ShouldRotateByDailyDetectsDifferentCalendarDay) {
    ScopedTempDir dir;
    const fs::path current = dir.path() / "daily.log";
    writeText(current, "entry");

    const auto entry_time = std::chrono::system_clock::now();
    ASSERT_TRUE(setAgeHours(current, 48));
    EXPECT_TRUE(shouldRotateByDaily(current, entry_time));

    ASSERT_TRUE(setAgeHours(current, 0));
    EXPECT_FALSE(shouldRotateByDaily(current, std::chrono::system_clock::now()));
}

TEST(FileSinkToolsTests, PruneTemplateFilesByAgeRemovesOnlyOldTemplateMatches) {
    ScopedTempDir dir;
    const fs::path logs_dir = dir.path() / "logs";
    const fs::path current = logs_dir / "app-2026-03-04.log";
    const fs::path old_match = logs_dir / "app-2026-03-01.log";
    const fs::path old_match_backup = logs_dir / "app-2026-03-01.log.1";
    const fs::path recent_match = logs_dir / "app-2026-03-03.log";
    const fs::path unrelated = logs_dir / "service-2026-03-01.log";

    ensureParentDirectory(current);
    writeText(current, "current");
    writeText(old_match, "old-match");
    writeText(old_match_backup, "old-match-backup");
    writeText(recent_match, "recent-match");
    writeText(unrelated, "unrelated");

    ASSERT_TRUE(setAgeHours(old_match, 72));
    ASSERT_TRUE(setAgeHours(old_match_backup, 72));
    ASSERT_TRUE(setAgeHours(recent_match, 1));
    ASSERT_TRUE(setAgeHours(unrelated, 72));

    pruneTemplateFilesByAge(current, "logs/app-{date}.log", 1);

    EXPECT_TRUE(fs::exists(current));
    EXPECT_FALSE(fs::exists(old_match));
    EXPECT_FALSE(fs::exists(old_match_backup));
    EXPECT_TRUE(fs::exists(recent_match));
    EXPECT_TRUE(fs::exists(unrelated));
}

// TODO: Add Windows-specific rotation tests for locked-file handling and replace-existing semantics.
