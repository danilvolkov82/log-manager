#include "system-log-sink-tools.h"

#include <stdexcept>
#include <string>

#if defined(__linux__)
#include <syslog.h>
#endif

namespace LogManager::Internal::Sinks {
namespace {
#if defined(__linux__)
void defaultSystemLogWrite(int priority, std::string_view message) {
    ::syslog(priority, "%s", std::string(message).c_str());
}
#else
void defaultSystemLogWrite(int, std::string_view) {
    throw std::runtime_error("SystemLogSink is supported only on Linux");
}
#endif

SystemLogWriteFn system_log_write_fn = defaultSystemLogWrite;
}

int
toSystemLogPriority(LogManager::LogLevel level) {
#if defined(__linux__)
    switch(level) {
    case LogManager::LogLevel::VERBOSE:
        return LOG_DEBUG;
    case LogManager::LogLevel::INFO:
        return LOG_INFO;
    case LogManager::LogLevel::WARN:
        return LOG_WARNING;
    case LogManager::LogLevel::ERROR:
        return LOG_ERR;
    case LogManager::LogLevel::FATAL:
        return LOG_CRIT;
    default:
        return LOG_INFO;
    }
#else
    (void)level;
    return 0;
#endif
}

void
sendToSystemLog(LogManager::LogLevel level, std::string_view message) {
    system_log_write_fn(toSystemLogPriority(level), message);
}

void
setSystemLogWriteFnForTests(SystemLogWriteFn write_fn) {
    system_log_write_fn = write_fn ? write_fn : defaultSystemLogWrite;
}
}
