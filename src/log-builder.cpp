/**
 * @file log-builder.cpp
 * @brief Implementation of LogBuilder.
 */
#include "log-builder.h"
#include "internal/logger.h"
#include "log-interface.h"
#include "sink-interface.h"

#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace LogManager;

/**
 * @brief Internal state container for LogBuilder.
 */
class LogBuilder::Impl {
private:
    std::vector<std::shared_ptr<ISink>> _sinks;
    bool _created{false};

public:
    Impl() = default;
    ~Impl() = default;

    /**
     * @brief Returns whether a logger has already been created.
     */
    bool isCreated() {
        return _created;
    }

    /**
     * @brief Marks builder state as finalized.
     */
    void markAsCreated() {
        _created = true;
    }

    /**
     * @brief Appends a sink to the builder state.
     * @param sink Sink to register.
     */
    void addSink(std::shared_ptr<ISink> sink) {
        if(_created) {
            throw std::runtime_error("Log is already created can't add sinks anymore");
        }

        if(!sink) {
            throw std::invalid_argument("sink must not be null");
        }

        _sinks.push_back(std::move(sink));
    }

    /**
     * @brief Copies configured sinks into the provided logger.
     * @param logger Logger instance to populate.
     */
    void fillSinks(std::shared_ptr<Internal::Logger> logger) {
        for (auto sink : _sinks) {
            logger->addSink(std::move(sink));
        }
    }
};

LogBuilder::LogBuilder()
    : _impl(std::make_unique<LogBuilder::Impl>())
{}

LogBuilder::~LogBuilder() = default;

LogBuilder&
LogBuilder::addSink(std::function<std::shared_ptr<ISink>()> sink_factory) {
    if(!sink_factory) {
        throw std::invalid_argument("sink factory must not be empty");
    }

    std::shared_ptr<ISink> sink = sink_factory();
    _impl->addSink(std::move(sink));

    return *this;
}

LogBuilder&
LogBuilder::addSinks(const std::vector<std::function<std::shared_ptr<ISink>()>>& sink_factories) {
    for (const auto &sink_factory : sink_factories) {
        addSink(sink_factory);
    }

    return *this;
}

std::shared_ptr<ILog>
LogBuilder::create() {
    if(_impl->isCreated()) {
        throw std::runtime_error("Logger is already created");
    }

    std::shared_ptr<Internal::Logger> logger = std::make_shared<Internal::Logger>();
    _impl->fillSinks(logger);
    _impl->markAsCreated();

    return std::static_pointer_cast<ILog>(std::move(logger));
}
