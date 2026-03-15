/**
 * @file file-sink.h
 * @brief File-based sink implementation with rotation support.
 */
//
// Created by danil on 28.02.2026 
//
#ifndef LOG_MANAGER_SINKS_FILE_SINK_H
#define LOG_MANAGER_SINKS_FILE_SINK_H
#include "sink-interface.h"
#include "tools.h"
#include "file-sink-config.h"
#include "file-sink-types.h"

#include <memory>
#include <cstdint>

#pragma once

namespace LogManager::Sinks::FileSink {
/**
 * @brief Writes log entries to a file.
 *
 * Configuration is provided through @ref configure using the JSON shape
 * supported by @ref LogManager::Sinks::FileSink::FileSinkConfig. A file sink
 * must be configured before the first call to @ref log and accepts only one
 * successful configuration for its lifetime.
 *
 * Template placeholders for message format (`format`):
 * - `{timestamp}`: Entry timestamp in the default date-time representation.
 * - `{level}`: Log level (`VERBOSE`, `INFO`, `WARN`, `ERROR`, `FATAL`).
 * - `{tag}`: Value from @ref LogManager::LogDetails::tag.
 * - `{message}`: Value from @ref LogManager::LogDetails::message.
 * - `{thread_id}`: Value from @ref LogManager::LogDetails::thread_id.
 * - `{exception}`: Exception text extracted from @ref LogManager::LogDetails::exception.
 *
 * Template placeholders for filename template (`filename_template`):
 * - `{yyyy}`: 4-digit year.
 * - `{MM}`: 2-digit month.
 * - `{dd}`: 2-digit day of month.
 * - `{HH}`: 2-digit hour (24-hour clock).
 * - `{mm}`: 2-digit minute.
 * - `{ss}`: 2-digit second.
 * - `{date}`: Date alias equivalent to `yyyy-MM-dd`.
 * - `{datetime}`: Date-time alias equivalent to `yyyyMMdd-HHmmss`.
 * - `{level}`: Log level text.
 * - `{tag}`: Log tag text.
 * - `{index}`: Rotation index for archived files.
 *
 * Internal state access is synchronized for a single sink instance.
 * Successful configuration can be observed through @ref isConfigured.
 */
class FileSink : public ISink {
private:
	class Impl;
	std::unique_ptr<Impl> _impl;
public:
	/**
	 * @brief Constructs a file sink with default configuration.
	 */
	FileSink();

	/**
	 * @brief Destroys the sink instance.
	 */
	~FileSink();

	/**
	 * @brief Applies file sink configuration from a JSON payload.
	 *
		 * Invalid payloads are rejected by the underlying config object and reported
		 * as warnings to the standard error stream. Once configuration succeeds,
		 * any later call throws.
	 *
	 * @param json_config JSON object string with file sink settings.
	 * @throws std::runtime_error If the sink was already configured successfully.
	 */
	void configure(const std::string &json_config) override;

	/**
	 * @brief Writes one log entry to the configured file target.
	 *
	 * Depending on the active configuration, this may rotate files before the
	 * entry is appended.
	 *
	 * @param log_entry Log entry to render and append.
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
} // LogManager::Sinks

#endif // LOG_MANAGER_SINKS_FILE_SINK_H
