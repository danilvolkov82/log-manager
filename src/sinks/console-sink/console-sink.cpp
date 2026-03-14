/**
 * @file console-sink.cpp
 * @brief Implementation of the console sink.
 */
#include "console-sink.h"

#include <iostream>
#include <mutex>
#include <stdexcept>

#include "sinks/general-sink-config.h"

#include "internal/log-message-tools.h"

using namespace LogManager::Sinks::ConsoleSink;
using namespace LogManager::Internal;

namespace {
std::mutex console_mutex{};
}

class ConsoleSink::Impl {
private:
    std::unique_ptr<GeneralSinkConfig> _config;
    bool _configured{false};
    std::mutex _log_mutex{};

    LogLevel getMinLevel() const {
        return _config->getMinLevel();
    }

    const std::string getFormat() const {
        return _config->getMessageFormat();
    }

public:
    Impl() : _config(std::make_unique<GeneralSinkConfig>()) {}
    ~Impl() = default;

    void configure(const std::string &json_config) {
        std::lock_guard<std::mutex> lock(_log_mutex);
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
            std::lock_guard<std::mutex> lock(_log_mutex);
            if (!_configured) {
                throw std::runtime_error("Sinks must be configured before usage");
            }

            if (static_cast<int>(log_entry.level) < static_cast<int>(this->getMinLevel())) {
                return;
            }

            message = renderMessageTemplate(this->getFormat(), log_entry);
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

