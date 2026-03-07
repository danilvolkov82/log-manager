#ifndef LOG_MANAGER_INTERNAL_LOG_MESSAGE_TOOLS
#define LOG_MANAGER_INTERNAL_LOG_MESSAGE_TOOLS
#pragma once

#include <exception>
#include <string>
#include <string_view>
#include <thread>

#include "tools.h"

/**
 * @file log-message-tools.h
 * @brief Internal helpers for rendering log message and filename templates.
 */
namespace LogManager::Internal {
/**
 * @brief Converts a thread id to string form for template rendering.
 * @param thread_id Thread identifier to serialize.
 * @return String representation of the provided thread id.
 */
std::string threadIdToString(const std::thread::id &thread_id);

/**
 * @brief Converts an exception pointer to display text.
 * @param exception Optional exception payload.
 * @return Empty string when exception is null, exception `what()` text for
 *         standard exceptions, or `"unknown exception"` otherwise.
 */
std::string exceptionToString(const std::exception_ptr &exception);

/**
 * @brief Converts a log level enum to its uppercase text form.
 * @param level Log level value.
 * @return One of `VERBOSE`, `INFO`, `WARN`, `ERROR`, `FATAL`, or `UNKNOWN`.
 */
std::string levelToString(LogManager::LogLevel level);

/**
 * @brief Renders filename template placeholders using a log entry payload.
 *
 * Supported placeholders include date/time parts (`{yyyy}`, `{MM}`, `{dd}`,
 * `{HH}`, `{mm}`, `{ss}`, `{date}`, `{datetime}`), level and tag.
 * When local time conversion fails, timestamp placeholders fall back to zeroed
 * values and a warning is written to the standard error stream.
 *
 * @param filename_template Template text to render.
 * @param entry Log entry source values.
 * @return Rendered filename string.
 */
std::string renderFilenameTemplate(std::string_view filename_template, const LogManager::LogDetails &entry);

/**
 * @brief Renders message template placeholders using a log entry payload.
 *
 * Supported placeholders include `{timestamp}`, `{level}`, `{tag}`,
 * `{message}`, `{thread_id}` and `{exception}`.
 * When local time conversion fails, `{timestamp}` falls back to
 * `0000-00-00 00:00:00` and a warning is written to the standard error stream.
 *
 * @param format_template Template text to render.
 * @param entry Log entry source values.
 * @return Rendered message line.
 */
std::string renderMessageTemplate(std::string_view format_template, const LogManager::LogDetails &entry);
}
#endif // LOG_MANAGER_INTERNAL_LOG_MESSAGE_TOOLS
