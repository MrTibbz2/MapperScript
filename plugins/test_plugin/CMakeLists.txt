cmake_minimum_required(VERSION 3.15)
project(test_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define your source files for the plugin here
set(SOURCES
        src/test_plugin.cpp
)

# Create a shared library (DLL/.so/.dylib depending on platform)
add_library(test_plugin SHARED ${SOURCES})

# Optionally, set any necessary include directories
target_include_directories(test_plugin PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/../../include
        # Add any other includes your plugin needs here
)

# Export symbols properly on Windows
if(WIN32)
    target_compile_definitions(test_plugin PRIVATE PLUGIN_EXPORTS)
endif()

set_target_properties(test_plugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
)

# Optional: set position independent code for Unix
set_target_properties(test_plugin PROPERTIES POSITION_INDEPENDENT_CODE ON)