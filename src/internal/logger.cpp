#include <mutex>
#include <stdexcept>
#include <vector>

#include "internal/logger.h"
#include "tools.h"

namespace LogManager::Internal {
class Logger::Impl {
private:
    std::mutex sinks_mutex;
    std::vector<std::shared_ptr<ISink>> sinks;

public:
    Impl() = default;
    ~Impl() = default;

    void log(const LogDetails &entry) {
        std::vector<std::shared_ptr<ISink>> sinks_snapshot;
        {
            std::lock_guard<std::mutex> lock(sinks_mutex);
            sinks_snapshot = sinks;
        }

        for(const auto &sink : sinks_snapshot) {
            if(!sink) {
                continue;
            }

            sink->log(entry);
        }
    }

    void addSink(std::shared_ptr<ISink> sink) {
        if(!sink) {
            throw std::invalid_argument("sink must not be null");
        }

        std::lock_guard<std::mutex> lock(sinks_mutex);
        sinks.push_back(std::move(sink));
    }
};
}

using namespace LogManager::Internal;
Logger::Logger()
    : ILog()
    , impl(std::make_unique<Logger::Impl>())
{}

Logger::~Logger() = default;

LOG_METHOD_MESSAGE(Logger::verbose) {
    LogDetails entry(LogLevel::VERBOSE, tag, message);
    impl->log(entry);
}

LOG_METHOD_MESSAGE(Logger::info) {
    LogDetails entry(LogLevel::INFO, tag, message);
    impl->log(entry);
}

LOG_METHOD_MESSAGE(Logger::warn) {
    LogDetails entry(LogLevel::WARN, tag, message);
    impl->log(entry);
}

LOG_METHOD_MESSAGE(Logger::error) {
    LogDetails entry(LogLevel::ERROR, tag, message);
    impl->log(entry);
}

LOG_METHOD_MESSAGE(Logger::fatal) {
    LogDetails entry(LogLevel::FATAL, tag, message);
    impl->log(entry);
}

LOG_METHOD_EXCEPTION(Logger::warn) {
    LogDetails entry(LogLevel::WARN, tag, e);
    impl->log(entry);
}

LOG_METHOD_EXCEPTION(Logger::error) {
    LogDetails entry(LogLevel::ERROR, tag, e);
    impl->log(entry);
}

LOG_METHOD_EXCEPTION(Logger::fatal) {
    LogDetails entry(LogLevel::FATAL, tag, e);
    impl->log(entry);
}

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::warn) {
    LogDetails entry(LogLevel::WARN, tag, message, e);
    impl->log(entry);
}

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::error) {
    LogDetails entry(LogLevel::ERROR, tag, message, e);
    impl->log(entry);
}

LOG_METHOD_MESSAGE_AND_EXCEPTION(Logger::fatal) {
    LogDetails entry(LogLevel::FATAL, tag, message, e);
    impl->log(entry);
}

void
Logger::addSink(std::shared_ptr<ISink> sink) {
    impl->addSink(std::move(sink));
}
