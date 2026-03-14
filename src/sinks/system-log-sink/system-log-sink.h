/**
 * @file system-log-sink.h
 * @brief System log sink implementation that writes rendered log entries to the Linux syslog service.
 */
#ifndef LOG_MANAGER_SINKS_SYSTEM_LOG_SINK_SYSTEM_LOG_SINK_H
#define LOG_MANAGER_SINKS_SYSTEM_LOG_SINK_SYSTEM_LOG_SINK_H
#pragma once

#include <memory>
#include <string>

#include "sink-interface.h"

namespace LogManager::Sinks::SystemLogSink {
/**
 * @brief Writes log entries to the Linux system log service.
 *
 * Configuration is provided through @ref configure using the JSON shape
 * supported by @ref LogManager::Sinks::GeneralSinkConfig. A system log sink
 * must be configured before the first call to @ref log and accepts only one
 * successful configuration for its lifetime.
 *
 * Message rendering supports the placeholders handled by
 * @ref LogManager::Internal::renderMessageTemplate, including `{timestamp}`,
 * `{level}`, `{tag}`, `{message}`, `{thread_id}`, and `{exception}`.
 *
 * Internal state access is synchronized for a single sink instance. Successful
 * configuration can be observed through @ref isConfigured.
 *
 * This sink targets Linux syslog. On unsupported platforms, delivery fails at
 * runtime when @ref log tries to emit a message.
 */
class SystemLogSink : public ISink {
private:
    class Impl;
    std::unique_ptr<Impl> _impl;

public:
    /**
     * @brief Constructs a system log sink with default configuration.
     */
    SystemLogSink();

    /**
     * @brief Destroys the sink instance.
     */
    ~SystemLogSink();

    /**
     * @brief Applies system log sink configuration from a JSON payload.
     *
     * Invalid payloads are rejected by the underlying config object and
     * reported as warnings to the standard error stream. Once configuration
     * succeeds, any later call throws.
     *
     * @param json_config JSON object string with system log sink settings.
     * @throws std::runtime_error If the sink was already configured
     * successfully.
     */
    void configure(const std::string &json_config) override;

    /**
     * @brief Writes one rendered log entry to the Linux system log service.
     *
     * The entry is filtered by the configured minimum log level before it is
     * rendered and sent.
     *
     * @param log_entry Log entry to render and send.
     * @throws std::runtime_error If the sink has not been configured yet.
     * @throws std::runtime_error If the current platform does not support syslog.
     */
    void log(const LogDetails &log_entry) override;

    /**
     * @brief Returns whether the sink has been configured successfully.
     *
     * This reports whether a prior @ref configure call completed successfully.
     * Failed configuration attempts leave the sink unconfigured.
     *
     * @return `true` after successful configuration, otherwise `false`.
     */
    bool isConfigured() override;
};
}

#endif // LOG_MANAGER_SINKS_SYSTEM_LOG_SINK_SYSTEM_LOG_SINK_H
