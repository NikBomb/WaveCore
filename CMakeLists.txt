cmake_minimum_required(VERSION 3.6)

project(
    WaveCore
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WAVECORE_ENABLE_TESTS "Build WaveCore tests" ON)
option(WAVECORE_ENABLE_SANITIZERS "Enable Sanitizers" ON)
option(WAVECORE_ENABLE_WARNINGS_AS_ERRORS "Treat Warnings as Errors" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_subdirectory(src)
add_subdirectory(apps)

if (WAVECORE_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

