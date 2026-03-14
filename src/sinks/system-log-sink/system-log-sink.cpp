/**
 * @file system-log-sink.cpp
 * @brief Implementation of the system log sink.
 */
#include "system-log-sink.h"

#include <mutex>
#include <stdexcept>
#include <string>

#include "internal/log-message-tools.h"
#include "internal/sinks/system-log-sink-tools.h"
#include "sinks/general-sink-config.h"

using namespace LogManager::Internal;
using namespace LogManager::Internal::Sinks;
using namespace LogManager::Sinks::SystemLogSink;

class SystemLogSink::Impl {
private:
    std::unique_ptr<LogManager::Sinks::GeneralSinkConfig> _config;
    bool _configured{false};
    std::mutex _log_mutex{};

    LogLevel getMinLevel() const {
        return _config->getMinLevel();
    }

    std::string getFormat() const {
        return _config->getMessageFormat();
    }

public:
    Impl()
        : _config(std::make_unique<LogManager::Sinks::GeneralSinkConfig>())
    {}

    ~Impl() = default;

    void configure(const std::string &json_config) {
        std::lock_guard<std::mutex> lock(_log_mutex);
        if(_configured) {
            throw std::runtime_error("Sinks can be configured only once");
        }

        if(_config->configure(json_config)) {
            _configured = true;
        }
    }

    void log(const LogDetails &log_entry) {
        std::string message;
        {
            std::lock_guard<std::mutex> lock(_log_mutex);
            if(!_configured) {
                throw std::runtime_error("Sinks must be configured before usage");
            }

            if(static_cast<int>(log_entry.level) < static_cast<int>(this->getMinLevel())) {
                return;
            }

            message = renderMessageTemplate(this->getFormat(), log_entry);
        }

        sendToSystemLog(log_entry.level, message);
    }
};

SystemLogSink::SystemLogSink()
    : _impl(std::make_unique<SystemLogSink::Impl>())
{}

SystemLogSink::~SystemLogSink() = default;

void
SystemLogSink::configure(const std::string &json_config) {
    _impl->configure(json_config);
}

void
SystemLogSink::log(const LogDetails &log_entry) {
    _impl->log(log_entry);
}
