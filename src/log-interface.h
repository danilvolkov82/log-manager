/**
 * @file log-interface.h
 * @brief Public logging interface used by logger implementations.
 */
#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H
#pragma once
#include <string_view>
#include <exception>

namespace LogManager {
/**
 * @def LOG_METHOD_MESSAGE
 * @brief Declares a log method that accepts tag and message.
 */
#ifndef LOG_METHOD_MESSAGE
#define LOG_METHOD_MESSAGE(name) \
    void name(std::string_view tag, std::string_view message)
#endif

/**
 * @def LOG_METHOD_MESSAGE_AND_EXCEPTION
 * @brief Declares a log method that accepts tag, message, and exception.
 */
#ifndef LOG_METHOD_MESSAGE_AND_EXCEPTION
#define LOG_METHOD_MESSAGE_AND_EXCEPTION(name) \
    void name(std::string_view tag, std::string_view message, const std::exception_ptr &e)
#endif

/**
 * @def LOG_METHOD_EXCEPTION
 * @brief Declares a log method that accepts tag and exception.
 */
#ifndef LOG_METHOD_EXCEPTION
#define LOG_METHOD_EXCEPTION(name) \
    void name(std::string_view tag, const std::exception_ptr &e)
#endif

/**
 * @brief Abstract logging interface.
 */
class ILog {
protected:
    /**
     * @brief Protected constructor for interface inheritance.
     */
    ILog() = default;
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ILog() = default;

    /// @brief Logs a verbose message.
    virtual LOG_METHOD_MESSAGE(verbose) = 0;
    /// @brief Logs an informational message.
    virtual LOG_METHOD_MESSAGE(info) = 0;
    /// @brief Logs a warning message.
    virtual LOG_METHOD_MESSAGE(warn) = 0;
    /// @brief Logs an error message.
    virtual LOG_METHOD_MESSAGE(error) = 0;
    /// @brief Logs a fatal message.
    virtual LOG_METHOD_MESSAGE(fatal) = 0;
    /// @brief Logs a warning exception.
    virtual LOG_METHOD_EXCEPTION(warn) = 0;
    /// @brief Logs an error exception.
    virtual LOG_METHOD_EXCEPTION(error) = 0;
    /// @brief Logs a fatal exception.
    virtual LOG_METHOD_EXCEPTION(fatal) = 0;
    /// @brief Logs a warning message with exception details.
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(warn) = 0;
    /// @brief Logs an error message with exception details.
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(error) = 0;
    /// @brief Logs a fatal message with exception details.
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(fatal) = 0;
};
}
#endif //LOG_INTERFACE_H
