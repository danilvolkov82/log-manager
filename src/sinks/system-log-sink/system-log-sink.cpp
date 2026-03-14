/**
 * @file system-log-sink.cpp
 * @brief Implementation of the system log sink.
 */
#include "system-log-sink.h"

#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>

#include "internal/log-message-tools.h"
#include "internal/sinks/system-log-sink-tools.h"
#include "sinks/general-sink-config.h"

using namespace LogManager::Internal;
using namespace LogManager::Internal::Sinks;
using namespace LogManager::Sinks::SystemLogSink;

namespace {
std::mutex system_log_mutex;
}

class SystemLogSink::Impl {
private:
    std::unique_ptr<LogManager::Sinks::GeneralSinkConfig> _config;
    bool _configured{false};
    std::shared_mutex _log_mutex;

public:
    Impl()
        : _config(std::make_unique<LogManager::Sinks::GeneralSinkConfig>())
    {}

    ~Impl() = default;

    bool isConfigured() {
        std::shared_lock<std::shared_mutex> shared_lock(_log_mutex);
        return _configured;
    }

    void configure(const std::string &json_config) {
        std::unique_lock<std::shared_mutex> lock(_log_mutex);
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
            std::shared_lock<std::shared_mutex> lock(_log_mutex);
            if (!_configured) {
                throw std::runtime_error("Sinks must be configured before usage");
            }

            if(static_cast<int>(log_entry.level) < static_cast<int>(_config->getMinLevel())) {
                return;
            }

            message = renderMessageTemplate(_config->getMessageFormat(), log_entry);
        }

        std::lock_guard<std::mutex> system_lock(system_log_mutex);
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

bool
SystemLogSink::isConfigured() {
    return _impl->isConfigured();
}