#include "file-sink-types.h"

namespace LogManager::Sinks::FileSink {
RotationMode 
operator|(RotationMode a, RotationMode b) {
    return static_cast<RotationMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

RotationMode 
operator&(RotationMode a, RotationMode b) {
    return static_cast<RotationMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

bool 
hasFlag(RotationMode value, RotationMode flag) {
    return (value & flag) != RotationMode::NONE;
}
}