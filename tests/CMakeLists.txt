add_executable(log-manager-tests
    log-builder-tests.cpp
    logger-tests.cpp
    console-sink-tests.cpp
    general-sink-config-tests.cpp
    file-sink-config-tests.cpp
    file-sink-tests.cpp
    file-sink-types-tests.cpp
    log-message-tools-tests.cpp
    file-sink-tools-tests.cpp
    time-tools-tests.cpp
)

if(TARGET GTest::gtest_main)
    set(LOG_MANAGER_GTEST_MAIN_TARGET GTest::gtest_main)
elseif(TARGET gtest_main)
    set(LOG_MANAGER_GTEST_MAIN_TARGET gtest_main)
else()
    message(FATAL_ERROR
        "GoogleTest main target not found. Ensure cmake/GTest.cmake is included before tests.")
endif()

target_link_libraries(log-manager-tests
    PRIVATE
    ${LOG_MANAGER_GTEST_MAIN_TARGET}
    log-manager
)

include(GoogleTest)
gtest_discover_tests(log-manager-tests)
