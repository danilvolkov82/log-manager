#ifndef LOG_MANAGER_INTERNAL_SINKS_FILE_SINK_TOOLS
#define LOG_MANAGER_INTERNAL_SINKS_FILE_SINK_TOOLS
#pragma once

/**
 * @file file-sink-tools.h
 * @brief Internal helpers for file sink write, rotation, and cleanup operations.
 */

#include <filesystem>
#include <stdint.h>
#include <string>
#include <chrono>

namespace LogManager::Internal::Sinks {
namespace fs = std::filesystem;

/**
 * @brief Appends one log line and a trailing newline to the target file.
 * @param current_file Path to the active log file.
 * @param line Rendered log line without a trailing newline.
 */
void appendLine(const fs::path &current_file, const std::string &line);

/**
 * @brief Removes old files that match the filename template in the active directory.
 * @param current_file Path to the active log file. The active file itself is never removed.
 * @param filename_template File template used by the sink (for example, "logs/app-{date}.log").
 * @param max_file_age_days Maximum allowed file age in days. `0` disables age pruning.
 */
void pruneTemplateFilesByAge(const fs::path &current_file, const std::string &filename_template, std::uint32_t max_file_age_days);

/**
 * @brief Ensures the parent directory of the target file exists.
 * @param current_file Path to the active log file.
 */
void ensureParentDirectory(const fs::path &current_file);

/**
 * @brief Checks whether appending data would exceed the configured size limit.
 * @param current_file Path to the active log file.
 * @param incoming_bytes Number of bytes that will be appended.
 * @param max_file_size Maximum allowed file size in bytes. `0` disables size-based rotation.
 * @return `true` when rotation should happen before appending, otherwise `false`.
 */
bool shouldRotateBySize(const fs::path &current_file, std::uint64_t incoming_bytes, std::uint64_t max_file_size);

/**
 * @brief Rotates the active file into numbered backups.
 * @param current_file Path to the active log file.
 * @param max_backup_files Backup retention limit:
 * `-1` means unlimited, `0` means no backups, positive values keep up to N backups.
 */
void rotateFile(const fs::path &current_file, std::int32_t max_backup_files);

/**
 * @brief Checks whether the log file should rotate because local calendar day changed.
 * @param current_file Path to the active log file.
 * @param entry_timestamp Timestamp of the incoming log entry.
 * @return `true` when file day differs from entry day, otherwise `false`.
 */
bool shouldRotateByDaily(const fs::path &current_file, std::chrono::system_clock::time_point entry_timestamp);
}
#endif //LOG_MANAGER_INTERNAL_SINKS_FILE_SINK_TOOLS
