/**
 * @file tools.h
 * @brief Core logging data types used across the library.
 */
#ifndef TOOLS_H
#define TOOLS_H
#pragma once

#include <string>
#include <string_view>
#include <exception>
#include <chrono>
#include <thread>

namespace LogManager {
/**
 * @brief Log severity levels.
 */
enum class LogLevel {
    VERBOSE,
    INFO,
    WARN,
    ERROR,
    FATAL,
    DISABLED
};

/**
 * @brief Immutable data captured for each log event.
 */
struct LogDetails {
    /// Severity of the log entry.
    LogLevel level{LogLevel::INFO};
    /// Logical source/category tag.
    std::string tag;
    /// Optional message payload.
    std::string message{};
    /// Optional associated exception.
    std::exception_ptr exception{};
    /// Timestamp when entry was created.
    std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
    /// Thread identifier of the emitting thread.
    std::thread::id thread_id{std::this_thread::get_id()};

    /**
     * @brief Creates a message-only log entry.
     * @param level Log severity.
     * @param tag Logical source/category tag.
     * @param message Message text.
     */
    LogDetails(LogLevel level, std::string_view tag, std::string_view message)
        : level(level)
        , tag(tag)
        , message(message)
    {}

    /**
     * @brief Creates an exception-only log entry.
     * @param level Log severity.
     * @param tag Logical source/category tag.
     * @param exception Exception payload.
     */
    LogDetails(LogLevel level, std::string_view tag, std::exception_ptr exception)
        : level(level)
        , tag(tag)
        , exception(std::move(exception))
    {}

    /**
     * @brief Creates a log entry with both message and exception.
     * @param level Log severity.
     * @param tag Logical source/category tag.
     * @param message Message text.
     * @param exception Exception payload.
     */
    LogDetails(LogLevel level, std::string_view tag, std::string_view message, std::exception_ptr exception)
        : level(level)
        , tag(tag)
        , message(message)
        , exception(std::move(exception))
    {}
};
}
#endif // TOOLS_H
