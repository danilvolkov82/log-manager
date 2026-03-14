//
// Created by danil on 28.02.2026 
//
#include "file-sink.h"

#include <mutex>
#include <shared_mutex>
#include <string>
#include <system_error>
#include <vector>
#include <unordered_set>

#include "internal/log-message-tools.h"
#include "internal/sinks/file-sink-tools.h"
#include "file-sink-config.h"
#include "file-sink-types.h"

using namespace LogManager::Internal;
using namespace LogManager::Internal::Sinks;

using namespace LogManager::Sinks::FileSink;

class FileSink::Impl {
private:
    std::unique_ptr<FileSinkConfig> _config;

    bool _configured{false};
    std::string _current_filename{};
    std::shared_mutex _log_mutex;
    std::mutex _file_mutex;
    std::unordered_set<std::string> _startup_checked_files;

public:
    Impl() : _config(std::make_unique<FileSinkConfig>()) {}
    ~Impl() = default;

    bool isConfigured() {
        std::shared_lock<std::shared_mutex> lock(_log_mutex);
        return _configured;
    }

    void configure(const std::string &json_config) {
        std::unique_lock<std::shared_mutex> lock(_log_mutex);
        if(_configured) {
            throw std::runtime_error("Sinks can be configured only once");
        }

        if(_config->configure(json_config)) {
            _startup_checked_files.clear();
            _configured = true;
        }
    }

    void log(const LogDetails &log_entry) {
        std::string current_line;
        std::string current_filename;
        std::string file_name_template;
        RotationMode rotation;
        int32_t max_backup_files;
        uint64_t max_file_size;
        uint32_t max_file_age;
        {
            std::shared_lock<std::shared_mutex> lock(_log_mutex);
            if (!_configured) {
                throw std::runtime_error("Sinks must be configured before usage");
            }

            if (static_cast<int>(log_entry.level) < static_cast<int>(_config->getMinLevel())) {
                return;
            }

            file_name_template = _config->getFileNameTemplate();
            current_filename = renderFilenameTemplate(file_name_template, log_entry);
            current_line = renderMessageTemplate(_config->getMessageFormat(), log_entry);

            rotation = _config->getRotation();
            max_backup_files = _config->getMaxBackupFiles();
            max_file_size = _config->getMaxFileSize();
            max_file_age = _config->getMaxFileAgeDays();
        }

        bool inserted_file;
        {
            std::unique_lock<std::shared_mutex> lock(_log_mutex);
            _current_filename = current_filename;
            inserted_file = _startup_checked_files.insert(_current_filename).second;
        }

        std::lock_guard<std::mutex> file_lock(_file_mutex);
        if (inserted_file && hasFlag(rotation, RotationMode::STARTUP)) {
            rotateFile(current_filename, max_backup_files);
        }

        if(hasFlag(rotation, RotationMode::DAILY) && shouldRotateByDaily(current_filename, log_entry.timestamp)) {
            rotateFile(current_filename, max_backup_files);
        }

        if(hasFlag(rotation, RotationMode::SIZE)) {
            const std::uint64_t incoming_bytes = static_cast<std::uint64_t>(current_line.size()) + 1U;
            if(shouldRotateBySize(current_filename, incoming_bytes, max_file_size)) {
                rotateFile(current_filename, max_backup_files);
            }
        }

        pruneTemplateFilesByAge(current_filename, file_name_template, max_file_age);

        ensureParentDirectory(current_filename);
        appendLine(current_filename, current_line);
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

bool
FileSink::isConfigured() {
    return _impl->isConfigured();
}
