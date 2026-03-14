/**
 * @file console-sink.h
 * @brief Console sink implementation that writes rendered log entries to the standard output stream.
 */
#ifndef LOG_MANAGER_SINKS_CONSOLE_SINK_CONSOLE_SINK_H
#define LOG_MANAGER_SINKS_CONSOLE_SINK_CONSOLE_SINK_H
#pragma once
#include <memory>
#include <string>

#include "sink-interface.h"

namespace LogManager::Sinks::ConsoleSink {
/**
 * @brief Writes log entries to the standard output stream.
 *
 * Configuration is provided through @ref configure using the JSON shape
 * supported by @ref LogManager::Sinks::GeneralSinkConfig. A console sink must
 * be configured before the first call to @ref log and accepts only one
 * successful configuration for its lifetime.
 *
 * Message rendering supports the placeholders handled by
 * @ref LogManager::Internal::renderMessageTemplate, including `{timestamp}`,
 * `{level}`, `{tag}`, `{message}`, `{thread_id}`, and `{exception}`.
 *
 * Internal state access is synchronized for a single sink instance. Successful
 * configuration can be observed through @ref isConfigured. Console writes are
 * also serialized across all console sink instances so that concurrently
 * emitted lines are not interleaved on the standard output stream.
 */
class ConsoleSink : public ISink {
private:
    class Impl;
    std::unique_ptr<Impl> _impl;

public:
    /**
     * @brief Constructs a console sink with default configuration.
     */
    ConsoleSink();

    /**
     * @brief Destroys the sink instance.
     */
    ~ConsoleSink();

    /**
     * @brief Applies console sink configuration from a JSON payload.
     *
     * Invalid payloads are rejected by the underlying config object and
     * reported as warnings to the standard error stream. Once configuration
     * succeeds, any later call throws.
     *
     * @param json_config JSON object string with console sink settings.
     * @throws std::runtime_error If the sink was already configured
     * successfully.
     */
    void configure(const std::string &json_config) override;

    /**
     * @brief Writes one rendered log entry to the standard output stream.
     *
     * The entry is filtered by the configured minimum log level before it is
     * rendered and written.
     *
     * @param log_entry Log entry to render and write.
     * @throws std::runtime_error If the sink has not been configured yet.
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
#endif // LOG_MANAGER_SINKS_CONSOLE_SINK_CONSOLE_SINK_H
