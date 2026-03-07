/**
 * @file log-message-tools.cpp
 * @brief Implementation of internal log-template rendering helpers.
 */
#include "log-message-tools.h"
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "time-tools.h"

namespace LogManager::Internal {
namespace {
void writeRenderWarning(const char *context, const std::exception &e) {
    std::cerr << "[WARN] LogMessageTools: failed to render " << context << ": "
              << e.what() << '\n';
}

void writeUnknownRenderWarning(const char *context) {
    std::cerr << "[WARN] LogMessageTools: failed to render " << context
              << ": unknown error\n";
}

void setFilenameTimestampFallbacks(std::unordered_map<std::string, std::string> &formats) {
    formats["{yyyy}"] = "0000";
    formats["{MM}"] = "00";
    formats["{dd}"] = "00";
    formats["{HH}"] = "00";
    formats["{mm}"] = "00";
    formats["{ss}"] = "00";
    formats["{date}"] = "0000-00-00";
    formats["{datetime}"] = "00000000-000000";
}

void setMessageTimestampFallback(std::unordered_map<std::string, std::string> &formats) {
    formats["{timestamp}"] = "0000-00-00 00:00:00";
}
}

std::string 
threadIdToString(const std::thread::id &thread_id) {
    std::ostringstream stream;
    stream << thread_id;
    return stream.str();
}

std::string 
exceptionToString(const std::exception_ptr &exception) {
    if(!exception) {
        return {};
    }

    try {
        std::rethrow_exception(exception);
    } catch(const std::exception &e) {
        return e.what();
    } catch(...) {
        return "unknown exception";
    }
}

std::string 
levelToString(LogManager::LogLevel level) {
    switch(level) {
    case LogManager::LogLevel::VERBOSE:
        return "VERBOSE";
    case LogManager::LogLevel::INFO:
        return "INFO";
    case LogManager::LogLevel::WARN:
        return "WARN";
    case LogManager::LogLevel::ERROR:
        return "ERROR";
    case LogManager::LogLevel::FATAL:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

inline void
clear_token(std::vector<std::string> &tokens, std::ostringstream &token, bool &token_clean) {
    tokens.push_back(token.str());
    token.clear();
    token.str("");
    token_clean = true;
}

inline std::vector<std::string> 
tokenize(std::string_view source) {
    std::vector<std::string> result;
    std::ostringstream token;
    bool token_clean = true;

    for(char c : source) {
        if(c == '{' && !token_clean) {
            clear_token(result, token, token_clean);
        }

        token << c;
        token_clean = false;
        if(c == '}') {
            clear_token(result, token, token_clean);
        }
    }

    if(!token_clean) {
        result.push_back(token.str());
    }

    return result;
}

std::string 
renderTemplate(std::string_view source, const std::unordered_map<std::string, std::string> &values) {
    std::vector<std::string> tokens = tokenize(source);
    std::ostringstream result;

    for(const auto token : tokens) {
        auto found_value = values.find(token);
        result << (found_value != values.cend() ? found_value->second : token);
    }

    return result.str();
}

std::string 
renderFilenameTemplate(std::string_view filename_template, const LogManager::LogDetails &entry) {
    std::unordered_map<std::string, std::string> formats = {
        {"{level}", levelToString(entry.level)},
        {"{tag}", entry.tag}
    };

    try {
        const std::tm tm = toLocalTime(entry.timestamp);
        formats["{yyyy}"] = formatTm(tm, "%Y");
        formats["{MM}"] = formatTm(tm, "%m");
        formats["{dd}"] = formatTm(tm, "%d");
        formats["{HH}"] = formatTm(tm, "%H");
        formats["{mm}"] = formatTm(tm, "%M");
        formats["{ss}"] = formatTm(tm, "%S");
        formats["{date}"] = formatTm(tm, "%Y-%m-%d");
        formats["{datetime}"] = formatTm(tm, "%Y%m%d-%H%M%S");
    } catch(const std::exception &e) {
        writeRenderWarning("filename template timestamp values", e);
        setFilenameTimestampFallbacks(formats);
    } catch(...) {
        writeUnknownRenderWarning("filename template timestamp values");
        setFilenameTimestampFallbacks(formats);
    }

    return renderTemplate(filename_template, formats);
}

std::string 
renderMessageTemplate(std::string_view format_template, const LogManager::LogDetails &entry) {
    std::unordered_map<std::string, std::string> formats = {
        {"{level}", levelToString(entry.level)},
        {"{tag}", entry.tag},
        {"{message}", entry.message},
        {"{thread_id}", threadIdToString(entry.thread_id)},
        {"{exception}", exceptionToString(entry.exception)}
    };

    try {
        const std::tm tm = toLocalTime(entry.timestamp);
        formats["{timestamp}"] = formatTm(tm, "%Y-%m-%d %H:%M:%S");
    } catch(const std::exception &e) {
        writeRenderWarning("message template timestamp", e);
        setMessageTimestampFallback(formats);
    } catch(...) {
        writeUnknownRenderWarning("message template timestamp");
        setMessageTimestampFallback(formats);
    }

    return renderTemplate(format_template, formats);
}
}
