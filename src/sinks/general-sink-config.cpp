#include "general-sink-config.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include <nlohmann/json.hpp>

using namespace LogManager;
using namespace LogManager::Sinks;

#define DEFAULT_MESSAGE_TEMPLATE "[{timestamp}] [{level} {tag}] {message} {exception}"
#define DEFAULT_LOG_LEVEL LogLevel::INFO

#define MESSAGE_FORMAT_FIELD "message_format"
#define LOG_LEVEL_FIELD "log_level"

namespace {
std::string normalizeLogLevelName(std::string_view value) {
    std::string normalized(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return normalized;
}

void validateLogLevelName(std::string_view value) {
    const std::string normalized = normalizeLogLevelName(value);

    if(normalized == "VERBOSE" || normalized == "INFO" || normalized == "WARN" ||
       normalized == "ERROR" || normalized == "FATAL" || normalized == "DISABLED") {
        return;
    }

    throw std::out_of_range("log_level name is not supported");
}

void validateLogLevelValue(const nlohmann::json &value) {
    if(value.is_string()) {
        validateLogLevelName(value.get<std::string>());
        return;
    }

    const int parsed_level = value.get<int>();
    constexpr int min_level = static_cast<int>(LogLevel::VERBOSE);
    constexpr int max_level = static_cast<int>(LogLevel::DISABLED);

    if(parsed_level < min_level || parsed_level > max_level) {
        throw std::out_of_range("log_level is out of supported range");
    }
}

LogLevel parseLogLevel(const nlohmann::json &value) {
    if(value.is_string()) {
        const std::string normalized = normalizeLogLevelName(value.get<std::string>());

        if(normalized == "VERBOSE") {
            return LogLevel::VERBOSE;
        }

        if(normalized == "INFO") {
            return LogLevel::INFO;
        }

        if(normalized == "WARN") {
            return LogLevel::WARN;
        }

        if(normalized == "ERROR") {
            return LogLevel::ERROR;
        }

        if(normalized == "FATAL") {
            return LogLevel::FATAL;
        }

        return LogLevel::DISABLED;
    }

    return static_cast<LogLevel>(value.get<int>());
}

void validateLogLevelField(const nlohmann::json &config) {
    if(config.contains(LOG_LEVEL_FIELD)) {
        validateLogLevelValue(config[LOG_LEVEL_FIELD]);
    }
}
}

class GeneralSinkConfig::Impl {
private:
    std::string _messageFormat{DEFAULT_MESSAGE_TEMPLATE};
    LogLevel _minLevel{DEFAULT_LOG_LEVEL};

public:
    Impl() = default;
    ~Impl() = default;

    std::string getMessageFormat() const {
        return _messageFormat;
    }

    LogLevel getMinLevel() const {
        return _minLevel;
    }

    void configure(const nlohmann::json &config) {
        std::string message_format = DEFAULT_MESSAGE_TEMPLATE;
        LogLevel min_level = DEFAULT_LOG_LEVEL;

        if(config.contains(MESSAGE_FORMAT_FIELD)) {
            message_format = config[MESSAGE_FORMAT_FIELD];
        }

        if(config.contains(LOG_LEVEL_FIELD)) {
            min_level = parseLogLevel(config[LOG_LEVEL_FIELD]);
        }

        _messageFormat = std::move(message_format);
        _minLevel = min_level;
    }
};

GeneralSinkConfig::GeneralSinkConfig()
    : _impl(std::make_unique<GeneralSinkConfig::Impl>()) {}

GeneralSinkConfig::~GeneralSinkConfig() = default;

std::string
GeneralSinkConfig::getMessageFormat() const {
    return _impl->getMessageFormat();
}

LogLevel
GeneralSinkConfig::getMinLevel() const {
    return _impl->getMinLevel();
}

bool
GeneralSinkConfig::configure(const std::string &json_config) {
    try {
        validate(json_config);
        processConfig(json_config);

        return true;
    } catch(const std::exception &e) {
        std::cerr << "[WARN] GeneralSinkConfig: failed to apply configuration: " << e.what() << '\n';
    } catch(...) {
        std::cerr << "[WARN] GeneralSinkConfig: failed to apply configuration: unknown error\n";
    }

    return false;
}

void
GeneralSinkConfig::validate(const std::string &json_config) const {
    auto parsed_config = nlohmann::json::parse(json_config);
    if (!parsed_config.is_object()) {
        throw std::invalid_argument("config must be a JSON object");
    }

    validateLogLevelField(parsed_config);
}

void
GeneralSinkConfig::processConfig(const std::string &json_config) {
    auto parsed_config = nlohmann::json::parse(json_config);
    _impl->configure(parsed_config);
}
