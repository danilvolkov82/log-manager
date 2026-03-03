include(FetchContent)

set(LOG_MANAGER_NLOHMANN_JSON_VERSION "3.7.3" CACHE STRING
    "nlohmann_json version used for FetchContent download")
set(nlohmann_json_POPULATED FALSE CACHE BOOL "Force nlohmann_json refetch each configure" FORCE)

# Ensure extracted archives get fresh timestamps to avoid stale rebuilds (CMP0135).
cmake_policy(SET CMP0135 NEW)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v${LOG_MANAGER_NLOHMANN_JSON_VERSION}/include.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Header-only usage: download sources but do not add upstream subdirectories.
FetchContent_GetProperties(nlohmann_json)

if(NOT nlohmann_json_POPULATED)
    if(POLICY CMP0169)
        cmake_policy(PUSH)
        cmake_policy(SET CMP0169 OLD) # allow Populate without adding upstream targets
    endif()
    FetchContent_Populate(nlohmann_json)
    if(POLICY CMP0169)
        cmake_policy(POP)
    endif()
endif()

if(NOT EXISTS "${nlohmann_json_SOURCE_DIR}/include/nlohmann/json.hpp")
    message(FATAL_ERROR
        "Downloaded nlohmann_json archive does not contain include/nlohmann/json.hpp.")
endif()

if(NOT TARGET nlohmann_json)
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json SYSTEM INTERFACE
        $<BUILD_INTERFACE:${nlohmann_json_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

if(NOT TARGET nlohmann_json::nlohmann_json)
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
endif()
