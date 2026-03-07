#ifndef LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_TYPES_H
#define LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_TYPES_H
#pragma once
#include <cstdint>

namespace LogManager::Sinks::FileSink {
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
}
#endif // LOG_MANAGER_SINKS_FILE_SINK_FILE_SINK_TYPES_H
