cmake_minimum_required(VERSION 3.15)
project(plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define your source files for the plugin here
set(SOURCES
        src/test_plugin.cpp
)

# Create a shared library (DLL/.so/.dylib depending on platform)
add_library(plugin SHARED ${SOURCES})

# Optionally, set any necessary include directories
target_include_directories(plugin PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/../../include
        # Add any other includes your plugin needs here
)

# Export symbols properly on Windows
if(WIN32)
    target_compile_definitions(plugin PRIVATE PLUGIN_EXPORTS)
endif()

set_target_properties(plugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/test_plugin"
)
add_custom_command(
        TARGET plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.lua
        $<TARGET_FILE_DIR:plugin>/plugin.lua
)
# Optional: set position independent code for Unix
set_target_properties(plugin PROPERTIES POSITION_INDEPENDENT_CODE ON)

set_target_properties(plugin PROPERTIES PREFIX "")