#ifndef LOG_MANAGER_INTERNAL_SINKS_SYSTEM_LOG_SINK_TOOLS
#define LOG_MANAGER_INTERNAL_SINKS_SYSTEM_LOG_SINK_TOOLS
#pragma once

/**
 * @file system-log-sink-tools.h
 * @brief Internal helpers for Linux system log delivery.
 */

#include <string_view>

#include "tools.h"

namespace LogManager::Internal::Sinks {
/**
 * @brief Callback type used to write one rendered entry to the platform log service.
 */
using SystemLogWriteFn = void(*)(int priority, std::string_view message);

/**
 * @brief Maps a library log level to a Linux syslog priority.
 * @param level Log level value to convert.
 * @return Syslog priority constant matching the provided level.
 */
int toSystemLogPriority(LogManager::LogLevel level);

/**
 * @brief Sends one rendered message to the Linux system log service.
 * @param level Library log level associated with the message.
 * @param message Rendered log line to send.
 * @throws std::runtime_error If the current platform does not support syslog.
 */
void sendToSystemLog(LogManager::LogLevel level, std::string_view message);

/**
 * @brief Overrides the low-level log writer used by @ref sendToSystemLog.
 *
 * This hook exists for tests and should normally remain unset in production.
 *
 * @param write_fn Replacement writer callback. Passing `nullptr` restores the default writer.
 */
void setSystemLogWriteFnForTests(SystemLogWriteFn write_fn);
}

#endif // LOG_MANAGER_INTERNAL_SINKS_SYSTEM_LOG_SINK_TOOLS
