/**
 * @file internal/logger.h
 * @brief Internal concrete logger implementation.
 */
#ifndef LOGGER_H
#define LOGGER_H
#pragma once

#include <memory>
#include <string_view>
#include <exception>

#include "../log-interface.h"
#include "../sink-interface.h"

namespace LogManager::Internal {
/**
 * @brief Concrete thread-safe logger that dispatches entries to sinks.
 */
class Logger : public ILog {
private:
    class Impl;
    std::unique_ptr<Impl> _impl;
public:
    /**
     * @brief Constructs the logger.
     */
    Logger();

    /**
     * @brief Destroys the logger.
     */
    ~Logger();

private:
    /// @copydoc LogManager::ILog::verbose
    LOG_METHOD_MESSAGE(verbose) override;
    /// @copydoc LogManager::ILog::info
    LOG_METHOD_MESSAGE(info) override;
    /// @copydoc LogManager::ILog::warn
    LOG_METHOD_MESSAGE(warn) override;
    /// @copydoc LogManager::ILog::error
    LOG_METHOD_MESSAGE(error) override;
    /// @copydoc LogManager::ILog::fatal
    LOG_METHOD_MESSAGE(fatal) override;
    /// @copydoc LogManager::ILog::warn
    LOG_METHOD_EXCEPTION(warn)override;
    /// @copydoc LogManager::ILog::error
    LOG_METHOD_EXCEPTION(error) override;
    /// @copydoc LogManager::ILog::fatal
    LOG_METHOD_EXCEPTION(fatal) override;
    /// @copydoc LogManager::ILog::warn
    LOG_METHOD_MESSAGE_AND_EXCEPTION(warn) override;
    /// @copydoc LogManager::ILog::error
    LOG_METHOD_MESSAGE_AND_EXCEPTION(error) override;
    /// @copydoc LogManager::ILog::fatal
    LOG_METHOD_MESSAGE_AND_EXCEPTION(fatal) override;

public:
    /**
     * @brief Registers a sink that receives future log entries.
     *
     * The sink is expected to be configured already, because sink
     * configuration is a one-time setup step handled before registration.
     *
     * @param sink Sink instance to register. Must be non-null.
     * @throws std::invalid_argument If @p sink is null.
     */
    void addSink(std::shared_ptr<ISink> sink);
};
}

#endif //LOGGER_H
