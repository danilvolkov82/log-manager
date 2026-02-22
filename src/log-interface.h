#ifndef LOG_INTERFACE_H
#define LOG_INTERFACE_H
#pragma once
#include <string_view>
#include <exception>

namespace LogManager {
#ifndef LOG_METHOD_MESSAGE
#define LOG_METHOD_MESSAGE(name) \
    void name(std::string_view tag, std::string_view message)
#endif
#ifndef LOG_METHOD_MESSAGE_AND_EXCEPTION
#define LOG_METHOD_MESSAGE_AND_EXCEPTION(name) \
    void name(std::string_view tag, std::string_view message, const std::exception_ptr &e)
#endif
#ifndef LOG_METHOD_EXCEPTION
#define LOG_METHOD_EXCEPTION(name) \
    void name(std::string_view tag, const std::exception_ptr &e)
#endif

class ILog {
protected:
    ILog() = default;
public:
    virtual ~ILog() = default;

    virtual LOG_METHOD_MESSAGE(verbose) = 0;
    virtual LOG_METHOD_MESSAGE(info) = 0;
    virtual LOG_METHOD_MESSAGE(warn) = 0;
    virtual LOG_METHOD_MESSAGE(error) = 0;
    virtual LOG_METHOD_MESSAGE(fatal) = 0;
    virtual LOG_METHOD_EXCEPTION(warn) = 0;
    virtual LOG_METHOD_EXCEPTION(error) = 0;
    virtual LOG_METHOD_EXCEPTION(fatal) = 0;
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(warn) = 0;
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(error) = 0;
    virtual LOG_METHOD_MESSAGE_AND_EXCEPTION(fatal) = 0;
};
}
#endif //LOG_INTERFACE_H