//
// Created by danil on 28.02.2026 
//
#ifndef LOG_MANAGER_SINKS_FILE_SINK_H
#define LOG_MANAGER_SINKS_FILE_SINK_H
#include "sink-interface.h"
#include "tools.h"

#include <memory>
#include <cstdint>

#pragma once

namespace LogManager::Sinks {
/**
 * @brief File rotation behavior flags.
 *
 * Flags can be combined with bitwise operators, for example:
 * `RotationMode::DAILY | RotationMode::SIZE`.
 */
enum class RotationMode : uint8_t {
	NONE = 0,
	DAILY = 1,
	SIZE = DAILY << 1,
	STARTUP = SIZE << 1
};

RotationMode operator|(RotationMode a, RotationMode b);
RotationMode operator&(RotationMode a, RotationMode b);
bool hasFlag(RotationMode value, RotationMode flag);

/**
 * @brief Writes log entries to a file.
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
 */
class FileSink : public ISink {
private:
	class Impl;
	std::unique_ptr<Impl> _impl;
public:
	FileSink();
	~FileSink();

	void configure(const std::string &json_config) override;
	void log(const LogDetails &log_entry) override;
};
} // LogManager::Sinks

#endif // LOG_MANAGER_SINKS_FILE_SINK_H
