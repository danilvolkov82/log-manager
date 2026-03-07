/**
 * @file general-sink-config.h
 * @brief Shared sink configuration for base formatting and level filtering.
 */
#ifndef LOG_MANAGER_SINKS_GENERAL_SINK_CONFIG
#define LOG_MANAGER_SINKS_GENERAL_SINK_CONFIG
#pragma once
#include <string>
#include <memory>

#include "tools.h"

namespace LogManager::Sinks {
/**
 * @brief Base configuration shared by sink implementations.
 *
 * `configure()` is the public entry point. It validates the payload through
 * @ref validate, then applies it through @ref processConfig. Any failure is
 * converted into a warning message on the standard error stream.
 *
 * The base implementation currently supports:
 * - `message_format`: output template used by the sink.
 * - `log_level`: minimum accepted level, as either an integer enum value or
 *   a case-insensitive name such as `INFO` or `warn`.
 *
 * Invalid configuration payloads are ignored and reported as warnings to the
 * standard error stream. On failure, the last valid configuration remains
 * active.
 *
 * Derived configs can override @ref validate to validate additional fields and
 * @ref processConfig to parse additional fields.
 * When overriding, they can call `GeneralSinkConfig::processConfig(...)` to
 * keep the shared base-field parsing after validation succeeds.
 */
class GeneralSinkConfig {
private:
    class Impl;
    std::unique_ptr<Impl> _impl;

public:
    /**
     * @brief Constructs a config with default values.
     */
    GeneralSinkConfig();

    /**
     * @brief Destroys the config instance.
     */
    virtual ~GeneralSinkConfig();

    /**
     * @brief Applies configuration from a JSON payload.
     *
     * This method calls @ref validate first and then @ref processConfig. It
     * catches any exception thrown by either step. Failures are reported as
     * warning messages to the standard error stream, and the last valid
     * configuration remains active.
     *
     * Example:
     * `{"message_format":"[{level}] {message}","log_level":"WARN"}`
     *
     * @param json_config JSON object string with base sink settings.
     * @return `true` when validation and parsing succeed, otherwise `false`.
     */
    bool configure(const std::string &json_config);

    /**
     * @brief Returns the configured minimum accepted log level.
     * @return Current minimum log level filter.
     */
    virtual LogLevel getMinLevel() const;

    /**
     * @brief Returns the configured message template.
     * @return Current message format string.
     */
    virtual std::string getMessageFormat() const;

protected:
    /**
     * @brief Validates a configuration payload for this config type.
     *
     * The base implementation checks that the payload is a JSON object and
     * validates the shared `log_level` field when present.
     *
     * @param json_config JSON object string with sink settings.
     * @throws std::exception When the payload is invalid for this config type.
     */
    virtual void validate(const std::string &json_config) const;

    /**
     * @brief Parses configuration fields for this config type.
     *
     * The base implementation parses the shared `message_format` and
     * `log_level` fields. Derived classes may override this method to parse
     * additional fields and may call `GeneralSinkConfig::processConfig(...)`
     * to preserve the shared parsing behavior.
     *
     * @param json_config JSON object string with sink settings.
     * @throws std::exception When the payload is invalid for this config type.
     */
    virtual void processConfig(const std::string &json_config);
};
}
#endif // LOG_MANAGER_SINKS_GENERAL_SINK_CONFIG
