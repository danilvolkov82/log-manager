#include <gtest/gtest.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "log-builder.h"
#include "sink-interface.h"
#include "tools.h"

namespace {
using namespace LogManager;

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

TEST(LogBuilderTests, AddSinkReturnsSameBuilderForChaining) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");

    LogBuilder &returned = builder.addSink([sink]() { return sink; });

    ASSERT_EQ(&returned, &builder);
}

TEST(LogBuilderTests, CreateBuildsLoggerThatForwardsMessagesToConfiguredSink) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    builder.addSink([sink]() { return sink; });

    auto logger = builder.create();
    ASSERT_NE(logger, nullptr);

    logger->info("test-tag", "test-message");

    ASSERT_EQ(sink->entries.size(), 1u);
    EXPECT_EQ(sink->entries[0].level, LogLevel::INFO);
    EXPECT_EQ(sink->entries[0].tag, "test-tag");
    EXPECT_EQ(sink->entries[0].message, "test-message");
}

TEST(LogBuilderTests, AddSinkThrowsWhenFactoryReturnsNull) {
    LogBuilder builder;
    EXPECT_THROW(builder.addSink([]() { return std::shared_ptr<ISink>{}; }), std::invalid_argument);
}

TEST(LogBuilderTests, CreateWithoutSinksStillBuildsUsableLogger) {
    LogBuilder builder;

    auto logger = builder.create();
    ASSERT_NE(logger, nullptr);
    EXPECT_NO_THROW(logger->warn("tag", "message"));
}

TEST(LogBuilderTests, AddSinkThrowsWhenFactoryIsEmpty) {
    LogBuilder builder;
    std::function<std::shared_ptr<ISink>()> empty_factory;

    EXPECT_THROW(builder.addSink(empty_factory), std::invalid_argument);
}

TEST(LogBuilderTests, AddSinkThrowsAfterLoggerCreation) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    builder.addSink([sink]() { return sink; });
    (void)builder.create();

    EXPECT_THROW(builder.addSink([sink]() { return sink; }), std::runtime_error);
}

TEST(LogBuilderTests, AddSinksReturnsSameBuilderForChaining) {
    LogBuilder builder;
    auto first = std::make_shared<RecordingSink>();
    auto second = std::make_shared<RecordingSink>();
    first->configure("{}");
    second->configure("{}");

    LogBuilder &returned = builder.addSinks({
        [first]() { return first; },
        [second]() { return second; }
    });

    ASSERT_EQ(&returned, &builder);
}

TEST(LogBuilderTests, AddSinksConfiguresAllSinksForCreatedLogger) {
    LogBuilder builder;
    auto first = std::make_shared<RecordingSink>();
    auto second = std::make_shared<RecordingSink>();
    first->configure("{}");
    second->configure("{}");

    builder.addSinks({
        [first]() { return first; },
        [second]() { return second; }
    });

    auto logger = builder.create();
    ASSERT_NE(logger, nullptr);
    logger->error("bulk-tag", "bulk-message");

    ASSERT_EQ(first->entries.size(), 1u);
    EXPECT_EQ(first->entries[0].level, LogLevel::ERROR);
    EXPECT_EQ(first->entries[0].tag, "bulk-tag");
    EXPECT_EQ(first->entries[0].message, "bulk-message");

    ASSERT_EQ(second->entries.size(), 1u);
    EXPECT_EQ(second->entries[0].level, LogLevel::ERROR);
    EXPECT_EQ(second->entries[0].tag, "bulk-tag");
    EXPECT_EQ(second->entries[0].message, "bulk-message");
}

TEST(LogBuilderTests, AddSinksThrowsWhenFactoryIsEmpty) {
    LogBuilder builder;
    std::function<std::shared_ptr<ISink>()> empty_factory;

    EXPECT_THROW(builder.addSinks({empty_factory}), std::invalid_argument);
}

TEST(LogBuilderTests, AddSinksThrowsWhenFactoryReturnsNull) {
    LogBuilder builder;

    EXPECT_THROW(
        builder.addSinks({[]() { return std::shared_ptr<ISink>{}; }}),
        std::invalid_argument
    );
}

TEST(LogBuilderTests, AddSinksThrowsAfterLoggerCreation) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    builder.addSink([sink]() { return sink; });
    (void)builder.create();

    EXPECT_THROW(builder.addSinks({[sink]() { return sink; }}), std::runtime_error);
}

TEST(LogBuilderTests, CreateThrowsWhenCalledTwice) {
    LogBuilder builder;

    auto first_logger = builder.create();
    ASSERT_NE(first_logger, nullptr);

    EXPECT_THROW(builder.create(), std::runtime_error);
}

TEST(LogBuilderTests, AddConfiguredSinkInstanceReturnsSameBuilderForChaining) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");

    LogBuilder &returned = builder.addSink(std::static_pointer_cast<ISink>(sink));

    ASSERT_EQ(&returned, &builder);
}

TEST(LogBuilderTests, AddConfiguredSinkInstanceBuildsLoggerThatForwardsMessages) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");

    builder.addSink(std::static_pointer_cast<ISink>(sink));
    auto logger = builder.create();

    ASSERT_NE(logger, nullptr);
    logger->info("direct-tag", "direct-message");

    ASSERT_EQ(sink->entries.size(), 1u);
    EXPECT_EQ(sink->entries[0].level, LogLevel::INFO);
    EXPECT_EQ(sink->entries[0].tag, "direct-tag");
    EXPECT_EQ(sink->entries[0].message, "direct-message");
}

TEST(LogBuilderTests, AddSinkInstanceThrowsWhenSinkIsNotConfigured) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();

    EXPECT_THROW(builder.addSink(std::static_pointer_cast<ISink>(sink)), std::runtime_error);
}

TEST(LogBuilderTests, AddSinkInstanceThrowsWhenSinkIsNull) {
    LogBuilder builder;

    EXPECT_THROW(builder.addSink(std::shared_ptr<ISink>{}), std::invalid_argument);
}

TEST(LogBuilderTests, AddSinkInstanceThrowsAfterLoggerCreation) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    (void)builder.create();

    EXPECT_THROW(builder.addSink(std::static_pointer_cast<ISink>(sink)), std::runtime_error);
}

TEST(LogBuilderTests, AddMultipleConfiguredSinkInstancesBuildsLoggerThatForwardsMessages) {
    LogBuilder builder;
    auto first = std::make_shared<RecordingSink>();
    auto second = std::make_shared<RecordingSink>();
    first->configure("{}");
    second->configure("{}");

    builder.addSinks({
        std::static_pointer_cast<ISink>(first),
        std::static_pointer_cast<ISink>(second)
    });

    auto logger = builder.create();
    ASSERT_NE(logger, nullptr);
    logger->error("direct-bulk-tag", "direct-bulk-message");

    ASSERT_EQ(first->entries.size(), 1u);
    EXPECT_EQ(first->entries[0].level, LogLevel::ERROR);
    EXPECT_EQ(first->entries[0].tag, "direct-bulk-tag");
    EXPECT_EQ(first->entries[0].message, "direct-bulk-message");

    ASSERT_EQ(second->entries.size(), 1u);
    EXPECT_EQ(second->entries[0].level, LogLevel::ERROR);
    EXPECT_EQ(second->entries[0].tag, "direct-bulk-tag");
    EXPECT_EQ(second->entries[0].message, "direct-bulk-message");
}

TEST(LogBuilderTests, AddMultipleSinkInstancesThrowsWhenAnySinkIsNotConfigured) {
    LogBuilder builder;
    auto configured = std::make_shared<RecordingSink>();
    auto not_configured = std::make_shared<RecordingSink>();
    configured->configure("{}");

    EXPECT_THROW(
        builder.addSinks({
            std::static_pointer_cast<ISink>(configured),
            std::static_pointer_cast<ISink>(not_configured)
        }),
        std::runtime_error
    );
}

TEST(LogBuilderTests, AddMultipleSinkInstancesThrowsWhenAnySinkIsNull) {
    LogBuilder builder;
    auto configured = std::make_shared<RecordingSink>();
    configured->configure("{}");

    EXPECT_THROW(
        builder.addSinks({
            std::static_pointer_cast<ISink>(configured),
            std::shared_ptr<ISink>{}
        }),
        std::invalid_argument
    );
}

TEST(LogBuilderTests, AddMultipleSinkInstancesThrowsAfterLoggerCreation) {
    LogBuilder builder;
    auto sink = std::make_shared<RecordingSink>();
    sink->configure("{}");
    (void)builder.create();

    EXPECT_THROW(
        builder.addSinks({std::static_pointer_cast<ISink>(sink)}),
        std::runtime_error
    );
}

TEST(LogBuilderTests, TemplatedAddSinkConfiguresAndRegistersSink) {
    LogBuilder builder;

    builder.addSink<RecordingSink>(R"({"kind":"recording"})");
    auto logger = builder.create();
    ASSERT_NE(logger, nullptr);

    logger->warn("templated-tag", "templated-message");
    SUCCEED();
}
