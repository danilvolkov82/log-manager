#ifndef TOOLS_H
#define TOOLS_H
#pragma once
#include <string>
#include <string_view>
#include <exception>
#include <chrono>
#include <thread>

namespace LogManager {
enum class LogLevel {
    VERBOSE,
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct LogDetails {
    LogLevel level{LogLevel::INFO};
    std::string tag;
    std::string message{};
    std::exception_ptr exception{};
    std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
    std::thread::id thread_id{std::this_thread::get_id()};

    LogDetails(LogLevel level, std::string_view tag, std::string_view message)
        : level(level)
        , tag(tag)
        , message(message)
    {}

    LogDetails(LogLevel level, std::string_view tag, std::exception_ptr exception)
        : level(level)
        , tag(tag)
        , exception(std::move(exception))
    {}

    LogDetails(LogLevel level, std::string_view tag, std::string_view message, std::exception_ptr exception)
        : level(level)
        , tag(tag)
        , message(message)
        , exception(std::move(exception))
    {}
};
}
#endif // TOOLS_H
