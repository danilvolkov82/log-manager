//
// Created by danil on 28.02.2026 
//
#include "file-sink.h"

#include <mutex>
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
    std::mutex _log_mutex{};
    std::unordered_set<std::string> _startup_checked_files{};

    LogLevel getMinLevel() const {
        return _config->getMinLevel();
    }

    const std::string getFileNameTemplate() const {
        return _config->getFileNameTemplate();
    }

    const std::string getFormat() const {
        return _config->getMessageFormat();
    }

    const RotationMode getRotation() const {
        return _config->getRotation();
    }

    const int32_t getMaxBackupFiles() const {
        return _config->getMaxBackupFiles();
    }

    const uint64_t getMaxFileSize() const {
        return _config->getMaxFileSize();
    }

    const uint32_t getMaxFileAgeDays() const {
        return _config->getMaxFileAgeDays();
    }

public:
    Impl() : _config(std::make_unique<FileSinkConfig>()) {}
    ~Impl() = default;

    void configure(const std::string &json_config) {
        std::lock_guard<std::mutex> lock(_log_mutex);
        if(_configured) {
            throw std::runtime_error("Sinks can be configured only once");
        }

        if(_config->configure(json_config)) {
            _startup_checked_files.clear();
            _configured = true;
        }
    }

    void log(const LogDetails &log_entry) {
        std::lock_guard<std::mutex> lock(_log_mutex);
        if(!_configured) {
            throw std::runtime_error("Sinks must be configured before usage");
        }

        if(static_cast<int>(log_entry.level) < static_cast<int>(this->getMinLevel())) {
            return;
        }

        _current_filename = renderFilenameTemplate(this->getFileNameTemplate(), log_entry);
        const std::string current_line = renderMessageTemplate(this->getFormat(), log_entry);

        const RotationMode rotation = this->getRotation();
        const int32_t max_backup_files = this->getMaxBackupFiles();
        if(_startup_checked_files.insert(_current_filename).second && hasFlag(rotation, RotationMode::STARTUP)) {
            rotateFile(_current_filename, max_backup_files);
        }

        if(hasFlag(rotation, RotationMode::DAILY) && shouldRotateByDaily(_current_filename, log_entry.timestamp)) {
            rotateFile(_current_filename, max_backup_files);
        }

        if(hasFlag(rotation, RotationMode::SIZE)) {
            const std::uint64_t incoming_bytes = static_cast<std::uint64_t>(current_line.size()) + 1U;
            if(shouldRotateBySize(_current_filename, incoming_bytes, this->getMaxFileSize())) {
                rotateFile(_current_filename, max_backup_files);
            }
        }

        pruneTemplateFilesByAge(_current_filename, this->getFileNameTemplate(), this->getMaxFileAgeDays());

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
