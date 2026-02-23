cmake_minimum_required(VERSION 3.10)

project(LogManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOG_MANAGER_SOURCES
    src/log-builder.cpp
    src/internal/logger.cpp
)

file(GLOB LOG_MANAGER_PUBLIC_HEADERS "src/*.h")
list(SORT LOG_MANAGER_PUBLIC_HEADERS)

add_library(log-manager STATIC ${LOG_MANAGER_SOURCES})
set_target_properties(log-manager PROPERTIES OUTPUT_NAME logmanager++)
target_include_directories(log-manager
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

add_library(log-manager-shared SHARED ${LOG_MANAGER_SOURCES})
set_target_properties(log-manager-shared PROPERTIES OUTPUT_NAME logmanager++)
target_include_directories(log-manager-shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS log-manager log-manager-shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${LOG_MANAGER_PUBLIC_HEADERS} DESTINATION include/log-manager)

set(SINGLE_HEADER_CONTENT "#ifndef LOG_MANAGER_H\n#define LOG_MANAGER_H\n\n")
foreach(HEADER_FILE ${LOG_MANAGER_PUBLIC_HEADERS})
    get_filename_component(HEADER_NAME ${HEADER_FILE} NAME)
    string(APPEND SINGLE_HEADER_CONTENT "#include \"log-manager/${HEADER_NAME}\"\n")
endforeach()
string(APPEND SINGLE_HEADER_CONTENT "\n#endif // LOG_MANAGER_H\n")

file(WRITE "${CMAKE_BINARY_DIR}/log-manager.h" "${SINGLE_HEADER_CONTENT}")
install(FILES "${CMAKE_BINARY_DIR}/log-manager.h" DESTINATION include)

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    include(cmake/GTest.cmake)
    add_subdirectory(tests)
endif()
