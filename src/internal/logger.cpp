/**
 * @file internal/logger.cpp
 * @brief Implementation of Internal::Logger.
 */
#include <mutex>
#include <stdexcept>
#include <vector>

#include "logger.h"
#include "../tools.h"

using namespace LogManager::Internal;

/**
 * @brief Thread-safe sink storage and dispatch logic for Logger.
 */
class Logger::Impl {
private:
    std::mutex _sinks_mutex;
    std::vector<std::shared_ptr<ISink>> _sinks;

public:
    Impl() = default;
    ~Impl() = default;

    /**
     * @brief Delivers a log entry to all registered sinks.
     * @param entry Entry to dispatch.
     */
    void log(const LogDetails &entry) {
        std::vector<std::shared_ptr<ISink>> sinks_snapshot;
        {
            std::lock_guard<std::mutex> lock(_sinks_mutex);
            sinks_snapshot = _sinks;
        }

        for(const auto &sink : sinks_snapshot) {
            if(!sink) {
                continue;
            }

            sink->log(entry);
        }
    }

    /**
     * @brief Registers a sink.
     * @param sink Sink to register.
     * @throws std::invalid_argument If @p sink is null.
     */
    void addSink(std::shared_ptr<ISink> sink) {
        if(!sink) {
            throw std::invalid_argument("sink must not be null");
        }

        std::lock_guard<std::mutex> lock(_sinks_mutex);
        _sinks.push_back(std::move(sink));
    }
};

Logger::Logger()
    : ILog()
    , _impl(std::make_unique<Logger::Impl>())
{}

Logger::~Logger() = default;

#define LOG_MESSAGE(level) \
    LogDetails entry(level, tag, message); \
    _impl->log(entry); \

LOG_METHOD_MESSAGE(Logger::verbose) {
    LOG_MESSAGE(LogLevel::VERBOSE)
}

LOG_METHOD_MESSAGE(Logger::info) {
    LOG_MESSAGE(LogLevel::INFO)
}

LOG_METHOD_MESSAGE(Logger::warn) {
    LOG_MESSAGE(LogLevel::WARN)
}

LOG_METHOD_MESSAGE(Logger::error) {
    LOG_MESSAGE(LogLevel::ERROR);
}

LOG_METHOD_MESSAGE(Logger::fatal) {
    LOG_MESSAGE(LogLevel::FATAL);
}
#undef LOG_MESSAGE

#define LOG_EXCEPTION(level) \
    LogDetails entry(level, tag, e); \
    _impl->log(entry); \

LOG_METHOD_EXCEPTION(Logger::warn) {
    LOG_EXCEPTION(LogLevel::WARN)
}

LOG_METHOD_EXCEPTION(Logger::error) {
    LOG_EXCEPTION(LogLevel::ERROR)
}

LOG_METHOD_EXCEPTION(Logger::fatal) {
    LOG_EXCEPTION(LogLevel::FATAL)
}

#undef LOG_EXCEPTION

#define LOG_MESSAGE_AND_EXCEPTION(level) \
    LogDetails entry(level, tag, message, e); \
    _impl->log(entry); \

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::warn) {
    LOG_MESSAGE_AND_EXCEPTION(LogLevel::WARN)
}

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::error) {
    LOG_MESSAGE_AND_EXCEPTION(LogLevel::ERROR)
}

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::fatal) {
    LOG_MESSAGE_AND_EXCEPTION(LogLevel::FATAL)
}
#undef LOG_MESSAGE_AND_EXCEPTION

void
Logger::addSink(std::shared_ptr<ISink> sink) {
    _impl->addSink(std::move(sink));
}
