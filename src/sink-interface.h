#ifndef SINK_INTERFACE_H
#define SINK_INTERFACE_H
#pragma once
#include "tools.h"
namespace LogManager {
class ISink {
protected:
    ISink() = default;
public:
    virtual ~ISink() = default;
    virtual void log(const LogDetails &log_entry) = 0;
};
}
#endif //SINK_INTERFACE_H