#include "file-sink-config.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string_view>

#include <nlohmann/json.hpp>

using namespace LogManager::Sinks::FileSink;

#define DEFAULT_FILE_NAME_TEMPLATE "logs/app-{date}.log"
#define DEFAULT_ROTATION RotationMode::NONE
#define DEFAULT_MAX_FILE_SIZE 0ULL
#define DEFAULT_MAX_BACKUP_FILES -1
#define DEFAULT_MAX_FILE_AGE_DAYS 0UL

#define FILE_NAME_TEMPLATE_FIELD "filename_template"
#define ROTATION_FIELD "rotation"
#define MAX_FILE_SIZE_FIELD "max_file_size"
#define MAX_BACKUP_FILES_FIELD "max_backup_files"
#define MAX_FILE_AGE_DAYS_FIELD "max_file_age_days"

namespace {
std::string normalizeRotationToken(std::string_view value) {
    std::string normalized(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return normalized;
}

std::string trimRotationToken(std::string_view value) {
    std::size_t start = 0;
    while(start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    std::size_t end = value.size();
    while(end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return std::string(value.substr(start, end - start));
}

RotationMode parseRotationToken(std::string_view token) {
    const std::string normalized = normalizeRotationToken(trimRotationToken(token));
    if(normalized == "NONE") {
        return RotationMode::NONE;
    }

    if(normalized == "DAILY") {
        return RotationMode::DAILY;
    }

    if(normalized == "SIZE") {
        return RotationMode::SIZE;
    }

    return RotationMode::STARTUP;
}

void validateRotationToken(std::string_view token) {
    const std::string trimmed_token = trimRotationToken(token);
    if(trimmed_token.empty()) {
        throw std::out_of_range("rotation token is empty");
    }

    const std::string normalized = normalizeRotationToken(trimmed_token);
    if(normalized == "NONE" || normalized == "DAILY" || normalized == "SIZE" || normalized == "STARTUP") {
        return;
    }

    throw std::out_of_range("rotation token is not supported");
}

void validateRotationValue(const nlohmann::json &value) {
    if(!value.is_string()) {
        throw std::invalid_argument("rotation must be a string");
    }

    const std::string source = value.get<std::string>();
    std::size_t token_start = 0;
    while(token_start <= source.size()) {
        const std::size_t delimiter = source.find('|', token_start);
        const std::size_t token_end = delimiter == std::string::npos ? source.size() : delimiter;
        validateRotationToken(std::string_view(source).substr(token_start, token_end - token_start));

        if(delimiter == std::string::npos) {
            break;
        }

        token_start = delimiter + 1;
    }
}

RotationMode parseRotation(const nlohmann::json &value) {
    RotationMode rotation = RotationMode::NONE;
    const std::string source = value.get<std::string>();

    std::size_t token_start = 0;
    while(token_start <= source.size()) {
        const std::size_t delimiter = source.find('|', token_start);
        const std::size_t token_end = delimiter == std::string::npos ? source.size() : delimiter;
        rotation = rotation | parseRotationToken(std::string_view(source).substr(token_start, token_end - token_start));

        if(delimiter == std::string::npos) {
            break;
        }

        token_start = delimiter + 1;
    }

    return rotation;
}
}

class FileSinkConfig::Impl {
private:
    std::string _filename_template{DEFAULT_FILE_NAME_TEMPLATE};
    RotationMode _rotation{DEFAULT_ROTATION};
    uint64_t _max_file_size{DEFAULT_MAX_FILE_SIZE};
    int32_t _max_backup_files{DEFAULT_MAX_BACKUP_FILES};
    uint32_t _max_file_age_days{DEFAULT_MAX_FILE_AGE_DAYS};

public:
    Impl() = default;
    ~Impl() = default;

    std::string getFileNameTemplate() const {
        return _filename_template;
    }

    RotationMode getRotation() const {
        return _rotation;
    }

    uint64_t getMaxFileSize() const {
        return _max_file_size;
    }

    int32_t getMaxBackupFiles() const {
        return _max_backup_files;
    }

    uint32_t getMaxFileAgeDays() const {
        return _max_file_age_days;
    }

    void configure(const nlohmann::json &config) {
        std::string file_name_template = DEFAULT_FILE_NAME_TEMPLATE;
        RotationMode rotation = DEFAULT_ROTATION;
        uint64_t max_file_size = DEFAULT_MAX_FILE_SIZE;
        int32_t max_backup_files = DEFAULT_MAX_BACKUP_FILES;
        uint32_t max_file_age_days = DEFAULT_MAX_FILE_AGE_DAYS;

        if(config.contains(FILE_NAME_TEMPLATE_FIELD)) {
            file_name_template = config[FILE_NAME_TEMPLATE_FIELD];
        }

        if(config.contains(ROTATION_FIELD)) {
            rotation = parseRotation(config[ROTATION_FIELD]);
        }

        if(config.contains(MAX_FILE_SIZE_FIELD)) {
            max_file_size = config[MAX_FILE_SIZE_FIELD];
        }

        if(config.contains(MAX_BACKUP_FILES_FIELD)) {
            max_backup_files = config[MAX_BACKUP_FILES_FIELD];
        }

        if(config.contains(MAX_FILE_AGE_DAYS_FIELD)) {
            max_file_age_days = config[MAX_FILE_AGE_DAYS_FIELD];
        }

        _filename_template = std::move(file_name_template);
        _rotation = rotation;
        _max_file_size = max_file_size;
        _max_backup_files = max_backup_files;
        _max_file_age_days = max_file_age_days;
    }
};

FileSinkConfig::FileSinkConfig() : _impl(std::make_unique<FileSinkConfig::Impl>()) {}
FileSinkConfig::~FileSinkConfig() = default;

std::string
FileSinkConfig::getFileNameTemplate() const {
    return _impl->getFileNameTemplate();
}

RotationMode
FileSinkConfig::getRotation() const {
    return _impl->getRotation();
}

uint64_t
FileSinkConfig::getMaxFileSize() const {
    return _impl->getMaxFileSize();
}

int32_t
FileSinkConfig::getMaxBackupFiles() const {
    return _impl->getMaxBackupFiles();
}

uint32_t
FileSinkConfig::getMaxFileAgeDays() const {
    return _impl->getMaxFileAgeDays();
}

void
FileSinkConfig::validate(const std::string &json_config) const {
    GeneralSinkConfig::validate(json_config);
    auto parsed_config = nlohmann::json::parse(json_config);

    if(parsed_config.contains(ROTATION_FIELD)) {
        validateRotationValue(parsed_config[ROTATION_FIELD]);
    }

    if(parsed_config.contains(MAX_FILE_SIZE_FIELD) && !parsed_config[MAX_FILE_SIZE_FIELD].is_number_unsigned()) {
        throw std::invalid_argument("max_file_size is not unsigned int64");
    }

    if(parsed_config.contains(MAX_BACKUP_FILES_FIELD) && !parsed_config[MAX_BACKUP_FILES_FIELD].is_number_integer()) {
        throw std::invalid_argument("max_backup_files is not int");
    }

    if (parsed_config.contains(MAX_FILE_AGE_DAYS_FIELD) && !parsed_config[MAX_FILE_AGE_DAYS_FIELD].is_number_unsigned()) {
        throw std::invalid_argument("max_file_age_days is not unsigned int32");
    }
}

void
FileSinkConfig::processConfig(const std::string &json_config) {
    auto parsed_config = nlohmann::json::parse(json_config);
    auto next_impl = std::make_unique<FileSinkConfig::Impl>(*_impl);
    next_impl->configure(parsed_config);

    GeneralSinkConfig::processConfig(json_config);
    _impl.swap(next_impl);
}
