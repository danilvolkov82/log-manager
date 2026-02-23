add_executable(log-manager-tests
    log-builder-tests.cpp
)

target_link_libraries(log-manager-tests
    GTest::gtest_main
    log-manager
)

include(GoogleTest)
gtest_discover_tests(log-manager-tests)
