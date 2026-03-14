#include <gtest/gtest.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "internal/logger.h"
#include "sink-interface.h"
#include "tools.h"

namespace {
using namespace LogManager;
using LogManager::Internal::Logger;

class RecordingSink : public ISink {
public:
    std::vector<LogDetails> entries;
    std::string last_config;

    void configure(const std::string &json_config) override {
        if(_configured) {
            throw std::runtime_error("Sinks can be configured only once");
        }

        last_config = json_config;
        _configured = true;
    }

    void log(const LogDetails &log_entry) override {
        if(!_configured) {
            throw std::runtime_error("Sinks must be configured before usage");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        entries.push_back(log_entry);
    }

    bool isConfigured() override {
        return _configured;
    }

private:
    bool _configured{false};
    std::mutex _mutex;
};
} // namespace

TEST(LoggerTests, AddSinkThrowsWhenSinkIsNull) {
    Logger logger;

    EXPECT_THROW(logger.addSink(nullptr), std::invalid_argument);
}

TEST(LoggerTests, MessageLogForwardsEntryToSink) {
    Logger logger;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    logger.addSink(sink);

    ILog &api = logger;
    api.info("test-tag", "test-message");

    ASSERT_EQ(sink->entries.size(), 1u);
    EXPECT_EQ(sink->entries[0].level, LogLevel::INFO);
    EXPECT_EQ(sink->entries[0].tag, "test-tag");
    EXPECT_EQ(sink->entries[0].message, "test-message");
    EXPECT_FALSE(static_cast<bool>(sink->entries[0].exception));
}

TEST(LoggerTests, ExceptionLogForwardsExceptionPayload) {
    Logger logger;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    logger.addSink(sink);

    ILog &api = logger;
    std::exception_ptr e = std::make_exception_ptr(std::runtime_error("boom"));
    api.warn("test-tag", e);

    ASSERT_EQ(sink->entries.size(), 1u);
    EXPECT_EQ(sink->entries[0].level, LogLevel::WARN);
    EXPECT_EQ(sink->entries[0].tag, "test-tag");
    EXPECT_TRUE(static_cast<bool>(sink->entries[0].exception));
    EXPECT_TRUE(sink->entries[0].message.empty());
}

TEST(LoggerTests, MessageAndExceptionLogForwardsAllPayload) {
    Logger logger;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    logger.addSink(sink);

    ILog &api = logger;
    std::exception_ptr e = std::make_exception_ptr(std::runtime_error("bad-state"));
    api.error("test-tag", "test-message", e);

    ASSERT_EQ(sink->entries.size(), 1u);
    EXPECT_EQ(sink->entries[0].level, LogLevel::ERROR);
    EXPECT_EQ(sink->entries[0].tag, "test-tag");
    EXPECT_EQ(sink->entries[0].message, "test-message");
    EXPECT_TRUE(static_cast<bool>(sink->entries[0].exception));
}

TEST(LoggerTests, LogEntryIsSentToAllRegisteredSinks) {
    Logger logger;
    auto first = std::make_shared<RecordingSink>();
    auto second = std::make_shared<RecordingSink>();
    first->configure("{}");
    second->configure("{}");
    logger.addSink(first);
    logger.addSink(second);

    ILog &api = logger;
    api.fatal("fanout-tag", "fanout-message");

    ASSERT_EQ(first->entries.size(), 1u);
    EXPECT_EQ(first->entries[0].level, LogLevel::FATAL);
    EXPECT_EQ(first->entries[0].tag, "fanout-tag");
    EXPECT_EQ(first->entries[0].message, "fanout-message");

    ASSERT_EQ(second->entries.size(), 1u);
    EXPECT_EQ(second->entries[0].level, LogLevel::FATAL);
    EXPECT_EQ(second->entries[0].tag, "fanout-tag");
    EXPECT_EQ(second->entries[0].message, "fanout-message");
}
