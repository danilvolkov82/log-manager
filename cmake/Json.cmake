include(FetchContent)

set(LOG_MANAGER_NLOHMANN_JSON_VERSION "3.7.3" CACHE STRING
    "nlohmann_json version used for FetchContent download")
option(LOG_MANAGER_USE_SYSTEM_NLOHMANN_JSON
    "Prefer system-provided nlohmann_json package before FetchContent"
    ON)
option(LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON
    "Force nlohmann_json download via FetchContent (skip system/local fallbacks)"
    OFF)
set(LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR "" CACHE PATH
    "Path to include directory that contains nlohmann/json.hpp")

function(log_manager_add_nlohmann_interface_target include_dir)
    if(NOT TARGET nlohmann_json)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json SYSTEM INTERFACE
            $<BUILD_INTERFACE:${include_dir}>
            $<INSTALL_INTERFACE:include>
        )
    endif()

    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    endif()
endfunction()

if(LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON AND LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR)
    message(STATUS
        "LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON=ON, ignoring "
        "LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR=${LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR}")
endif()

if(NOT LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON AND LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR)
    if(NOT EXISTS "${LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR}/nlohmann/json.hpp")
        message(FATAL_ERROR
            "LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR must contain nlohmann/json.hpp. "
            "Current value: ${LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR}")
    endif()
    log_manager_add_nlohmann_interface_target("${LOG_MANAGER_NLOHMANN_JSON_INCLUDE_DIR}")
    return()
endif()

if(LOG_MANAGER_USE_SYSTEM_NLOHMANN_JSON AND NOT LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON)
    find_package(nlohmann_json CONFIG QUIET)
endif()

if(TARGET nlohmann_json::nlohmann_json)
    return()
endif()

if(NOT LOG_MANAGER_FORCE_FETCH_NLOHMANN_JSON)
    # Offline fallback: reuse a previously fetched local copy when available.
    find_path(LOG_MANAGER_NLOHMANN_JSON_AUTO_INCLUDE_DIR
        NAMES nlohmann/json.hpp
        PATH_SUFFIXES include single_include
        PATHS
            "${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src"
            "${CMAKE_SOURCE_DIR}/build/_deps/nlohmann_json-src"
            "${CMAKE_SOURCE_DIR}/build-tests/_deps/nlohmann_json-src"
        NO_DEFAULT_PATH
    )
    if(LOG_MANAGER_NLOHMANN_JSON_AUTO_INCLUDE_DIR)
        log_manager_add_nlohmann_interface_target("${LOG_MANAGER_NLOHMANN_JSON_AUTO_INCLUDE_DIR}")
        return()
    endif()
endif()

# Ensure extracted archives get fresh timestamps to avoid stale rebuilds (CMP0135).
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v${LOG_MANAGER_NLOHMANN_JSON_VERSION}/include.zip
    URL_HASH SHA256=87b5884741427220d3a33df1363ae0e8b898099fbc59f1c451113f6732891014
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

log_manager_add_nlohmann_interface_target("${nlohmann_json_SOURCE_DIR}/include")
