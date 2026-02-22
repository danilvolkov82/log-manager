include(FetchContent)

# Ensure extracted archives get fresh timestamps to avoid stale rebuilds (CMP0135).
cmake_policy(SET CMP0135 NEW)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Suppress deprecation warnings coming from the googletest subproject only.
set(_prev_warn_deprecated "${CMAKE_WARN_DEPRECATED}")
set(CMAKE_WARN_DEPRECATED OFF)
FetchContent_MakeAvailable(googletest)
set(CMAKE_WARN_DEPRECATED "${_prev_warn_deprecated}")
