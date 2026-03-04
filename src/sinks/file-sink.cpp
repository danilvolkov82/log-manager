//
// Created by danil on 28.02.2026 
//
#include "file-sink.h"

#include <mutex>
#include <string>
#include <system_error>
#include <vector>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "internal/log-message-tools.h"
#include "internal/sinks/file-sink-tools.h"

using namespace LogManager::Internal;
using namespace LogManager::Internal::Sinks;

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
