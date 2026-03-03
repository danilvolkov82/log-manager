/**
 * @file sink-interface.h
 * @brief Sink abstraction for handling log entries.
 */
#ifndef SINK_INTERFACE_H
#define SINK_INTERFACE_H
#pragma once
#include <string>

#include "tools.h"

namespace LogManager {
/**
 * @brief Abstract log sink.
 */
class ISink {
protected:
    /**
     * @brief Protected constructor for interface inheritance.
     */
    ISink() = default;
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ISink() = default;
    virtual void configure(const std::string &json_config);

    /**
     * @brief Writes a log entry to the sink.
     * @param log_entry Entry to process.
     */
    virtual void log(const LogDetails &log_entry) = 0;
};
}
#endif //SINK_INTERFACE_H
