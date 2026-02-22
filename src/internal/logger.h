#ifndef LOGGER_H
#define LOGGER_H
#pragma once
#include <memory>
#include <string_view>
#include <exception>

#include "log-interface.h"
#include "sink-interface.h"

namespace LogManager::Internal {
class Logger : public ILog {
private:
    class Impl;
    std::unique_ptr<Impl> impl;
public:
    Logger();
    ~Logger();

private:
    LOG_METHOD_MESSAGE(verbose) override;
    LOG_METHOD_MESSAGE(info) override;
    LOG_METHOD_MESSAGE(warn) override;
    LOG_METHOD_MESSAGE(error) override;
    LOG_METHOD_MESSAGE(fatal) override;
    LOG_METHOD_EXCEPTION(warn)override;
    LOG_METHOD_EXCEPTION(error) override;
    LOG_METHOD_EXCEPTION(fatal) override;
    LOG_METHOD_MESSAGE_AND_EXCEPTION(warn) override;
    LOG_METHOD_MESSAGE_AND_EXCEPTION(error) override;
    LOG_METHOD_MESSAGE_AND_EXCEPTION(fatal) override;

public:
    void addSink(std::shared_ptr<ISink> sink);
};
}

#endif //LOGGER_H