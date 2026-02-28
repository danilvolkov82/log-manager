# Prefer an already-installed package first.
find_package(GTest QUIET CONFIG)
if(NOT TARGET GTest::gtest)
    find_package(GTest QUIET)
endif()

# Older integrations may expose non-namespaced targets only.
if(NOT TARGET GTest::gtest AND TARGET gtest)
    add_library(GTest::gtest ALIAS gtest)
endif()
if(NOT TARGET GTest::gtest_main AND TARGET gtest_main)
    add_library(GTest::gtest_main ALIAS gtest_main)
endif()

if(NOT TARGET GTest::gtest_main)
    include(FetchContent)

    # Ensure extracted archives get fresh timestamps to avoid stale rebuilds (CMP0135).
    cmake_policy(SET CMP0135 NEW)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_GetProperties(googletest)

    if(FETCHCONTENT_FULLY_DISCONNECTED
       AND NOT EXISTS "${googletest_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR
            "GoogleTest is required for BUILD_TESTS=ON, but downloads are disabled "
            "(FETCHCONTENT_FULLY_DISCONNECTED=ON) and no local googletest source is available. "
            "Install GTest or rerun CMake with FETCHCONTENT_FULLY_DISCONNECTED=OFF.")
    endif()

    # Suppress deprecation warnings coming from the googletest subproject only.
    set(_prev_warn_deprecated "${CMAKE_WARN_DEPRECATED}")
    set(CMAKE_WARN_DEPRECATED OFF)
    FetchContent_MakeAvailable(googletest)
    set(CMAKE_WARN_DEPRECATED "${_prev_warn_deprecated}")

    # Older googletest versions may expose non-namespaced targets only.
    if(NOT TARGET GTest::gtest AND TARGET gtest)
        add_library(GTest::gtest ALIAS gtest)
    endif()
    if(NOT TARGET GTest::gtest_main AND TARGET gtest_main)
        add_library(GTest::gtest_main ALIAS gtest_main)
    endif()
endif()

if(NOT TARGET GTest::gtest_main)
    message(FATAL_ERROR
        "GoogleTest main target is unavailable. Expected GTest::gtest_main or gtest_main.")
endif()
