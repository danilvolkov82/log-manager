#include <gtest/gtest.h>

#include "sinks/file-sink/file-sink-types.h"

namespace {
using namespace LogManager::Sinks::FileSink;
}

TEST(FileSinkTypesTests, BitwiseOrCombinesRotationFlags) {
    const RotationMode combined = RotationMode::DAILY | RotationMode::SIZE | RotationMode::STARTUP;

    EXPECT_TRUE(hasFlag(combined, RotationMode::DAILY));
    EXPECT_TRUE(hasFlag(combined, RotationMode::SIZE));
    EXPECT_TRUE(hasFlag(combined, RotationMode::STARTUP));
}

TEST(FileSinkTypesTests, BitwiseAndKeepsOnlyCommonFlags) {
    const RotationMode left = RotationMode::DAILY | RotationMode::SIZE;
    const RotationMode right = RotationMode::SIZE | RotationMode::STARTUP;
    const RotationMode common = left & right;

    EXPECT_FALSE(hasFlag(common, RotationMode::DAILY));
    EXPECT_TRUE(hasFlag(common, RotationMode::SIZE));
    EXPECT_FALSE(hasFlag(common, RotationMode::STARTUP));
}

TEST(FileSinkTypesTests, HasFlagReturnsFalseForNoneAndMissingFlags) {
    EXPECT_FALSE(hasFlag(RotationMode::NONE, RotationMode::DAILY));
    EXPECT_FALSE(hasFlag(RotationMode::DAILY, RotationMode::SIZE));
    EXPECT_FALSE(hasFlag(RotationMode::SIZE, RotationMode::STARTUP));
}
