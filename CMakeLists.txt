cmake_minimum_required(VERSION 3.16)

project(LogManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOG_MANAGER_SOURCES
    src/log-builder.cpp
    src/internal/logger.cpp
    src/internal/time-tools.cpp
    src/internal/log-message-tools.cpp
    src/internal/sinks/file-sink-tools.cpp
    src/internal/sinks/system-log-sink-tools.cpp
    src/sinks/general-sink-config.cpp
    src/sinks/file-sink/file-sink-types.cpp
    src/sinks/file-sink/file-sink-config.cpp
    src/sinks/file-sink/file-sink.cpp
    src/sinks/console-sink/console-sink.cpp
    src/sinks/system-log-sink/system-log-sink.cpp
)

include(cmake/Json.cmake)

file(GLOB LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src" "src/*.h")
set(LOG_MANAGER_GENERAL_SINK_PUBLIC_HEADER "sinks/general-sink-config.h")
file(GLOB_RECURSE LOG_MANAGER_SINK_PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/src" "src/sinks/**/*.h")
set(LOG_MANAGER_PUBLIC_HEADERS
    ${LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS}
    ${LOG_MANAGER_SINK_PUBLIC_HEADERS}
)
list(SORT LOG_MANAGER_PUBLIC_HEADERS)

set(LOG_MANAGER_ROOT_UMBRELLA_HEADERS
    ${LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS}
    ${LOG_MANAGER_GENERAL_SINK_PUBLIC_HEADER}
)
set(LOG_MANAGER_FILE_SINK_UMBRELLA_HEADERS
    sinks/file-sink/file-sink.h
    sinks/file-sink/file-sink-config.h
    sinks/file-sink/file-sink-types.h
)
set(LOG_MANAGER_CONSOLE_SINK_UMBRELLA_HEADERS
    sinks/console-sink/console-sink.h
)
set(LOG_MANAGER_SYSTEM_LOG_SINK_UMBRELLA_HEADERS
    sinks/system-log-sink/system-log-sink.h
)

set(LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS_ABS)
foreach(HEADER_FILE ${LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS})
    list(APPEND LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS_ABS "${CMAKE_CURRENT_SOURCE_DIR}/src/${HEADER_FILE}")
endforeach()
set(LOG_MANAGER_GENERAL_SINK_PUBLIC_HEADER_ABS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/${LOG_MANAGER_GENERAL_SINK_PUBLIC_HEADER}")

add_library(log-manager STATIC ${LOG_MANAGER_SOURCES})
set_target_properties(log-manager PROPERTIES OUTPUT_NAME logmanager++)
target_include_directories(log-manager
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/log-manager>
)
target_link_libraries(log-manager
    PRIVATE
        nlohmann_json::nlohmann_json
)

add_library(log-manager-shared SHARED ${LOG_MANAGER_SOURCES})
set_target_properties(log-manager-shared PROPERTIES OUTPUT_NAME logmanager++)
target_include_directories(log-manager-shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/log-manager>
)
target_link_libraries(log-manager-shared
    PRIVATE
        nlohmann_json::nlohmann_json
)

install(TARGETS log-manager log-manager-shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${LOG_MANAGER_TOP_LEVEL_PUBLIC_HEADERS_ABS}
    DESTINATION include/log-manager)
install(FILES "${LOG_MANAGER_GENERAL_SINK_PUBLIC_HEADER_ABS}" DESTINATION include/log-manager/sinks)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/sinks/file-sink/" DESTINATION include/log-manager/sinks/file-sink
    FILES_MATCHING
    PATTERN "*.h"
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/sinks/console-sink/" DESTINATION include/log-manager/sinks/console-sink
    FILES_MATCHING
    PATTERN "*.h"
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/sinks/system-log-sink/" DESTINATION include/log-manager/sinks/system-log-sink
    FILES_MATCHING
    PATTERN "*.h"
)

set(SINGLE_HEADER_CONTENT "#ifndef LOG_MANAGER_H\n#define LOG_MANAGER_H\n\n")
foreach(HEADER_FILE ${LOG_MANAGER_ROOT_UMBRELLA_HEADERS})
    string(APPEND SINGLE_HEADER_CONTENT "#include \"log-manager/${HEADER_FILE}\"\n")
endforeach()
string(APPEND SINGLE_HEADER_CONTENT "\n#endif // LOG_MANAGER_H\n")

set(FILE_SINK_SINGLE_HEADER_CONTENT "#ifndef FILE_SINK_H\n#define FILE_SINK_H\n\n")
foreach(HEADER_FILE ${LOG_MANAGER_FILE_SINK_UMBRELLA_HEADERS})
    string(APPEND FILE_SINK_SINGLE_HEADER_CONTENT "#include \"log-manager/${HEADER_FILE}\"\n")
endforeach()
string(APPEND FILE_SINK_SINGLE_HEADER_CONTENT "\n#endif // FILE_SINK_H\n")

set(CONSOLE_SINK_SINGLE_HEADER_CONTENT "#ifndef CONSOLE_SINK_H\n#define CONSOLE_SINK_H\n\n")
foreach(HEADER_FILE ${LOG_MANAGER_CONSOLE_SINK_UMBRELLA_HEADERS})
    string(APPEND CONSOLE_SINK_SINGLE_HEADER_CONTENT "#include \"log-manager/${HEADER_FILE}\"\n")
endforeach()
string(APPEND CONSOLE_SINK_SINGLE_HEADER_CONTENT "\n#endif // CONSOLE_SINK_H\n")

set(SYSTEM_LOG_SINK_SINGLE_HEADER_CONTENT
    "#ifndef SYSTEM_LOG_SINK_H\n#define SYSTEM_LOG_SINK_H\n\n")
foreach(HEADER_FILE ${LOG_MANAGER_SYSTEM_LOG_SINK_UMBRELLA_HEADERS})
    string(APPEND SYSTEM_LOG_SINK_SINGLE_HEADER_CONTENT "#include \"log-manager/${HEADER_FILE}\"\n")
endforeach()
string(APPEND SYSTEM_LOG_SINK_SINGLE_HEADER_CONTENT "\n#endif // SYSTEM_LOG_SINK_H\n")

file(WRITE "${CMAKE_BINARY_DIR}/log-manager.h" "${SINGLE_HEADER_CONTENT}")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/sinks")
file(WRITE "${CMAKE_BINARY_DIR}/sinks/file-sink.h" "${FILE_SINK_SINGLE_HEADER_CONTENT}")
file(WRITE "${CMAKE_BINARY_DIR}/sinks/console-sink.h" "${CONSOLE_SINK_SINGLE_HEADER_CONTENT}")
file(WRITE "${CMAKE_BINARY_DIR}/sinks/system-log-sink.h" "${SYSTEM_LOG_SINK_SINGLE_HEADER_CONTENT}")
install(FILES "${CMAKE_BINARY_DIR}/log-manager.h" DESTINATION include)
install(FILES
    "${CMAKE_BINARY_DIR}/sinks/file-sink.h"
    "${CMAKE_BINARY_DIR}/sinks/console-sink.h"
    "${CMAKE_BINARY_DIR}/sinks/system-log-sink.h"
    DESTINATION include/sinks)

option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    include(cmake/GTest.cmake)
    add_subdirectory(tests)
endif()
