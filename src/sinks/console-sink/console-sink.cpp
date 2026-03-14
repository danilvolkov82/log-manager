/**
 * @file console-sink.cpp
 * @brief Implementation of the console sink.
 */
#include "console-sink.h"

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>

#include "sinks/general-sink-config.h"

#include "internal/log-message-tools.h"

using namespace LogManager::Sinks::ConsoleSink;
using namespace LogManager::Internal;

namespace {
std::mutex console_mutex;
}

class ConsoleSink::Impl {
private:
    std::unique_ptr<GeneralSinkConfig> _config;
    bool _configured{false};
    std::shared_mutex _log_mutex;

public:
    Impl() : _config(std::make_unique<GeneralSinkConfig>()) {}
    ~Impl() = default;

    bool isConfigured() {
        std::shared_lock<std::shared_mutex> lock(_log_mutex);
        return _configured;
    }

    void configure(const std::string &json_config) {
        std::unique_lock<std::shared_mutex> lock(_log_mutex);
        if (_configured)  {
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

            if (static_cast<int>(log_entry.level) < static_cast<int>(_config->getMinLevel())) {
                return;
            }

            message = renderMessageTemplate(_config->getMessageFormat(), log_entry);
        }

        std::lock_guard<std::mutex> console_lock(console_mutex);
        std::cout << message << '\n';
    }
};

ConsoleSink::ConsoleSink() : _impl(std::make_unique<ConsoleSink::Impl>()) {}
ConsoleSink::~ConsoleSink() = default;

void
ConsoleSink::configure(const std::string &json_config) {
    _impl->configure(json_config);
}

void
ConsoleSink::log(const LogDetails &log_entry) {
    _impl->log(log_entry);
}

bool
ConsoleSink::isConfigured() {
    return _impl->isConfigured();
}
