//
// Created by danil on 28.02.2026 
//
#include "file-sink.h"

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>
#include <string>
#include <system_error>
#include <vector>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "internal/time-tools.h"
#include "internal/log-message-tools.h"

using namespace LogManager::Internal;

namespace {
namespace fs = std::filesystem;

struct BackupFile {
    std::uint32_t index;
    fs::path path;
};

fs::path makeBackupPath(const fs::path &current_file, std::uint32_t index) {
    return fs::path(current_file.string() + "." + std::to_string(index));
}

std::vector<BackupFile> collectBackups(const fs::path &current_file) {
    std::vector<BackupFile> backups;

    fs::path directory = current_file.parent_path();
    if(directory.empty()) {
        directory = ".";
    }

    std::error_code ec;
    if(!fs::exists(directory, ec) || !fs::is_directory(directory, ec)) {
        return backups;
    }

    const std::string base_name = current_file.filename().string();
    fs::directory_iterator it(directory, ec);
    const fs::directory_iterator end;
    while(!ec && it != end) {
        const fs::path path = it->path();
        ++it;

        if(!fs::is_regular_file(path, ec)) {
            continue;
        }

        const std::string file_name = path.filename().string();
        const std::string prefix = base_name + ".";
        if(file_name.rfind(prefix, 0) != 0) {
            continue;
        }

        const std::string suffix = file_name.substr(prefix.size());
        if(suffix.empty()) {
            continue;
        }

        std::uint32_t index = 0;
        const char *begin = suffix.data();
        const char *end_ptr = suffix.data() + suffix.size();
        const auto result = std::from_chars(begin, end_ptr, index);
        if(result.ec != std::errc() || result.ptr != end_ptr || index == 0) {
            continue;
        }

        backups.push_back(BackupFile{index, path});
    }

    return backups;
}

void pruneBackups(const fs::path &current_file, std::int32_t max_backup_files) {
    if(max_backup_files < 0) {
        return;
    }

    const auto backups = collectBackups(current_file);
    for(const auto &backup : backups) {
        if(backup.index > static_cast<std::uint32_t>(max_backup_files)) {
            std::error_code ec;
            fs::remove(backup.path, ec);
        }
    }
}

void rotateFile(const fs::path &current_file, std::int32_t max_backup_files) {
    std::error_code ec;
    if(max_backup_files == 0) {
        fs::remove(current_file, ec);
        pruneBackups(current_file, 0);
        return;
    }

    if(!fs::exists(current_file, ec)) {
        pruneBackups(current_file, max_backup_files);
        return;
    }

    auto backups = collectBackups(current_file);
    std::sort(backups.begin(), backups.end(), [](const BackupFile &a, const BackupFile &b) {
        return a.index > b.index;
    });

    for(const auto &backup : backups) {
        fs::path target = makeBackupPath(current_file, backup.index + 1);
        std::error_code move_ec;
        if(fs::exists(target, move_ec)) {
            fs::remove(target, move_ec);
        }
        fs::rename(backup.path, target, move_ec);
    }

    fs::path first_backup = makeBackupPath(current_file, 1);
    if(fs::exists(first_backup, ec)) {
        fs::remove(first_backup, ec);
    }
    fs::rename(current_file, first_backup, ec);

    pruneBackups(current_file, max_backup_files);
}

void ensureParentDirectory(const fs::path &current_file) {
    const fs::path parent = current_file.parent_path();
    if(parent.empty()) {
        return;
    }

    std::error_code ec;
    fs::create_directories(parent, ec);
}

std::uint64_t getFileSize(const fs::path &file_path) {
    std::error_code ec;
    if(!fs::exists(file_path, ec) || !fs::is_regular_file(file_path, ec)) {
        return 0;
    }

    const auto size = fs::file_size(file_path, ec);
    if(ec) {
        return 0;
    }

    return static_cast<std::uint64_t>(size);
}

bool shouldRotateBySize(const fs::path &current_file, std::uint64_t incoming_bytes, std::uint64_t max_file_size) {
    if(max_file_size == 0) {
        return false;
    }

    const std::uint64_t current_size = getFileSize(current_file);
    if(current_size >= max_file_size) {
        return true;
    }

    return incoming_bytes > (max_file_size - current_size);
}

std::chrono::system_clock::time_point fileTimeToSystemClock(fs::file_time_type file_time) {
    const auto file_now = fs::file_time_type::clock::now();
    const auto sys_now = std::chrono::system_clock::now();
    const auto delta = file_time - file_now;
    return sys_now + std::chrono::duration_cast<std::chrono::system_clock::duration>(delta);
}

int toLocalDateKey(std::chrono::system_clock::time_point timestamp) {
    const std::tm tm = toLocalTime(timestamp);
    const int year = tm.tm_year + 1900;
    const int month = tm.tm_mon + 1;
    const int day = tm.tm_mday;

    return (year * 10000) + (month * 100) + day;
}

bool tryToLocalDateKey(std::chrono::system_clock::time_point timestamp, int &date_key) {
    try {
        date_key = toLocalDateKey(timestamp);
        return true;
    } catch(...) {
        return false;
    }
}

bool shouldRotateByDaily(const fs::path &current_file, std::chrono::system_clock::time_point entry_timestamp) {
    std::error_code ec;
    if(!fs::exists(current_file, ec) || !fs::is_regular_file(current_file, ec)) {
        return false;
    }

    const fs::file_time_type last_write_time = fs::last_write_time(current_file, ec);
    if(ec) {
        return false;
    }

    int file_date = 0;
    int entry_date = 0;
    if(!tryToLocalDateKey(fileTimeToSystemClock(last_write_time), file_date) ||
       !tryToLocalDateKey(entry_timestamp, entry_date)) {
        return false;
    }

    return file_date != entry_date;
}

bool isOlderThanDays(const fs::path &file_path, std::uint32_t max_file_age_days, std::chrono::system_clock::time_point now) {
    if(max_file_age_days == 0) {
        return false;
    }

    std::error_code ec;
    if(!fs::exists(file_path, ec) || !fs::is_regular_file(file_path, ec)) {
        return false;
    }

    const fs::file_time_type last_write_time = fs::last_write_time(file_path, ec);
    if(ec) {
        return false;
    }

    const auto file_time = fileTimeToSystemClock(last_write_time);
    if(file_time >= now) {
        return false;
    }

    const auto age = now - file_time;
    const auto max_age = std::chrono::hours(24) * max_file_age_days;
    return age > max_age;
}

std::string tokenToRegex(std::string_view token) {
    if(token == "yyyy") {
        return R"(\d{4})";
    }
    if(token == "MM" || token == "dd" || token == "HH" || token == "mm" || token == "ss") {
        return R"(\d{2})";
    }
    if(token == "date") {
        return R"(\d{4}-\d{2}-\d{2})";
    }
    if(token == "datetime") {
        return R"(\d{8}-\d{6})";
    }
    if(token == "level") {
        return R"([A-Z]+)";
    }
    if(token == "tag") {
        return R"([^/\\]+)";
    }
    if(token == "index") {
        return R"(\d+)";
    }

    return {};
}

std::string escapeRegexChar(char c) {
    switch(c) {
    case '.':
    case '^':
    case '$':
    case '|':
    case '(':
    case ')':
    case '[':
    case ']':
    case '*':
    case '+':
    case '?':
    case '{':
    case '}':
    case '\\':
        return std::string("\\") + c;
    default:
        return std::string(1, c);
    }
}

std::regex buildFilenameRegex(const std::string &filename_template) {
    const std::string name_template = fs::path(filename_template).filename().string();
    std::string pattern = "^";

    for(std::size_t i = 0; i < name_template.size();) {
        if(name_template[i] == '{') {
            const std::size_t close = name_template.find('}', i + 1);
            if(close != std::string::npos) {
                const std::string token = name_template.substr(i + 1, close - i - 1);
                const std::string token_pattern = tokenToRegex(token);
                if(!token_pattern.empty()) {
                    pattern += token_pattern;
                    i = close + 1;
                    continue;
                }
            }
        }

        pattern += escapeRegexChar(name_template[i]);
        ++i;
    }

    // Match rotated backups too: "<rendered-name>.<index>".
    pattern += R"((\.\d+)?)$)";
    return std::regex(pattern);
}

void pruneTemplateFilesByAge(const fs::path &current_file, const std::string &filename_template, std::uint32_t max_file_age_days) {
    if(max_file_age_days == 0) {
        return;
    }

    fs::path directory = current_file.parent_path();
    if(directory.empty()) {
        directory = ".";
    }

    std::error_code ec;
    if(!fs::exists(directory, ec) || !fs::is_directory(directory, ec)) {
        return;
    }

    std::regex filename_regex = buildFilenameRegex(filename_template);
    const auto now = std::chrono::system_clock::now();
    fs::directory_iterator it(directory, ec);
    const fs::directory_iterator end;
    while(!ec && it != end) {
        const fs::path candidate = it->path();
        ++it;

        if(!fs::is_regular_file(candidate, ec)) {
            continue;
        }

        if(candidate.filename() == current_file.filename()) {
            continue;
        }

        if(!std::regex_match(candidate.filename().string(), filename_regex)) {
            continue;
        }

        if(isOlderThanDays(candidate, max_file_age_days, now)) {
            std::error_code ec;
            fs::remove(candidate, ec);
        }
    }
}

void appendLine(const fs::path &current_file, const std::string &line) {
    std::ofstream output(current_file, std::ios::out | std::ios::app);
    if(!output.is_open()) {
        return;
    }

    output << line << '\n';
}
} // namespace

namespace LogManager::Sinks {
RotationMode operator|(RotationMode a, RotationMode b) {
    return static_cast<RotationMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

RotationMode operator&(RotationMode a, RotationMode b) {
    return static_cast<RotationMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

bool hasFlag(RotationMode value, RotationMode flag) {
    return (value & flag) != RotationMode::NONE;
}
}

using namespace LogManager::Sinks;
namespace json = nlohmann::json;

class FileSink::Impl {
private:
    std::string _format{"[{timestamp}] [{level} {tag}] {message} {exception}"};
    std::string _filename_template{"logs/app-{date}.log"};
    RotationMode _rotation{RotationMode::NONE};
    uint64_t _max_file_size{0};
    std::int32_t _max_backup_files{-1};
    uint32_t _max_file_age_days{0};
    LogLevel _min_level{LogLevel::VERBOSE};

    std::string _current_filename;
    std::mutex _log_mutex;
    std::unordered_set<std::string> _startup_checked_files;

public:
    Impl() = default;
    ~Impl() = default;

    void configure(const std::string &json_config) {
    }

    void log(const LogDetails &log_entry) {
        std::lock_guard<std::mutex> lock(_log_mutex);
        if(static_cast<int>(log_entry.level) < static_cast<int>(_min_level)) {
            return;
        }

        _current_filename = renderFilenameTemplate(_filename_template, log_entry);
        const std::string current_line = renderMessageTemplate(_format, log_entry);

        if(_startup_checked_files.insert(_current_filename).second && hasFlag(_rotation, RotationMode::STARTUP)) {
            rotateFile(_current_filename, _max_backup_files);
        }

        if(hasFlag(_rotation, RotationMode::DAILY) && shouldRotateByDaily(_current_filename, log_entry.timestamp)) {
            rotateFile(_current_filename, _max_backup_files);
        }

        if(hasFlag(_rotation, RotationMode::SIZE)) {
            const std::uint64_t incoming_bytes = static_cast<std::uint64_t>(current_line.size()) + 1U;
            if(shouldRotateBySize(_current_filename, incoming_bytes, _max_file_size)) {
                rotateFile(_current_filename, _max_backup_files);
            }
        }

        pruneTemplateFilesByAge(_current_filename, _filename_template, _max_file_age_days);

        ensureParentDirectory(_current_filename);
        appendLine(_current_filename, current_line);
    }
};

FileSink::FileSink()
    : _impl(std::make_unique<FileSink::Impl>()) {}

FileSink::~FileSink() = default;

void
FileSink::configure(const std::string &json_config) {
    _impl->configure(json_config);
}

void
FileSink::log(const LogDetails &log_entry) {
    _impl->log(log_entry);
}
