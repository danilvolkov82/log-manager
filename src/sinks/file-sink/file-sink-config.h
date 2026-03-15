/**
 * @file file-sink-config.h
 * @brief Configuration object for file sink specific settings.
 */
#ifndef LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_CONFIG_H
#define LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_CONFIG_H
#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "../general-sink-config.h"
#include "file-sink-types.h"

namespace LogManager::Sinks::FileSink {
/**
 * @brief Configuration for the file sink.
 *
 * In addition to the shared fields handled by
 * @ref LogManager::Sinks::GeneralSinkConfig, this config validates and parses:
 * - `filename_template`
 * - `rotation`
 * - `max_file_size`
 * - `max_backup_files`
 * - `max_file_age_days`
 *
 * The `rotation` field accepts a string containing one or more rotation names
 * separated by `|`, for example `DAILY|SIZE`. Supported names are `NONE`,
 * `DAILY`, `SIZE`, and `STARTUP`. Integer rotation values are not supported.
 *
 * The configuration flow follows the base class contract:
 * @ref validate checks file-sink-specific fields first, then
 * @ref processConfig applies the validated values while preserving the last
 * valid state if any step fails.
 */
class FileSinkConfig : public GeneralSinkConfig {
private:
    class Impl;
    std::unique_ptr<Impl> _impl;
public:
    /**
     * @brief Constructs a config with default file sink settings.
     */
    FileSinkConfig();

    /**
     * @brief Destroys the config instance.
     */
    ~FileSinkConfig();

    /**
     * @brief Returns the configured file name template.
     * @return Current filename template string.
     */
    std::string getFileNameTemplate() const;

    /**
     * @brief Returns the configured rotation flags.
     * @return Current file rotation mode flags.
     */
    RotationMode getRotation() const;

    /**
     * @brief Returns the configured maximum file size in bytes.
     * @return Current size limit before rotation.
     */
    uint64_t getMaxFileSize() const;

    /**
     * @brief Returns the configured backup file limit.
     * @return Current maximum number of retained backup files.
     */
    int32_t getMaxBackupFiles() const;

    /**
     * @brief Returns the configured retention age in days.
     * @return Current maximum file age for pruning.
     */
    uint32_t getMaxFileAgeDays() const;

protected:
    /**
     * @brief Validates file sink specific fields.
     *
     * The base validation still checks the shared JSON object shape and
     * `log_level` field. This override adds validation for `rotation`,
     * `max_file_size`, `max_backup_files`, and `max_file_age_days`.
     *
     * @param json_config JSON object string with file sink settings.
     * @throws std::exception When the payload is invalid.
     */
    void validate(const std::string &json_config) const override;

    /**
     * @brief Parses file sink specific and shared configuration fields.
     *
     * This method assumes @ref validate has already succeeded. Derived parsing
     * keeps the base `message_format` and `log_level` handling by delegating to
     * @ref LogManager::Sinks::GeneralSinkConfig::processConfig.
     *
     * @param json_config JSON object string with file sink settings.
     * @throws std::exception When the payload is invalid.
     */
    void processConfig(const std::string &json_config) override;
};
}
#endif // LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_CONFIG_H
