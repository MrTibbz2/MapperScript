#/plugins/math_consumer/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(math_consumer_plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define plugin source files
set(SOURCES
        src/math_consumer.cpp
)

# Create shared library (plugin)
add_library(math_consumer_plugin SHARED ${SOURCES})

# Include directories - minimal dependencies for plugin development
target_include_directories(math_consumer_plugin PRIVATE
        ../../include                           # Only main app headers (for pluginContext)
        ../../vendored/lua/include             # Windows Lua headers
        ../../vendored/linux_lua/include       # Linux Lua headers
        ../../vendored/osx_lua/include         # macOS Lua headers
        ../../vendored/sol2/include            # Sol2 headers (needed for pluginContext)
)

# Platform-specific Lua linking
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Link against Lua import library - expects lua54.dll in main app directory
    target_link_libraries(math_consumer_plugin PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../vendored/lua/liblua54.dll.a")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Link against shared Lua library in main app directory
    target_link_directories(math_consumer_plugin PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../vendored/linux_lua")
    target_link_libraries(math_consumer_plugin PRIVATE lua54)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Link against shared Lua library in main app directory
    target_link_directories(math_consumer_plugin PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../../vendored/osx_lua")
    target_link_libraries(math_consumer_plugin PRIVATE lua)
endif()

# Windows DLL export definitions
if(WIN32)
    target_compile_definitions(math_consumer_plugin PRIVATE PLUGIN_EXPORTS)
endif()

# Plugin output settings - output as plugin.so/dll
set_target_properties(math_consumer_plugin PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/math_consumer"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/math_consumer"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/math_consumer"
        POSITION_INDEPENDENT_CODE ON
        PREFIX ""  # Remove lib prefix on Unix
        OUTPUT_NAME "plugin"  # Output as plugin.so/dll
)

# Copy plugin Lua script alongside compiled plugin
add_custom_command(TARGET math_consumer_plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin.lua
        $<TARGET_FILE_DIR:math_consumer_plugin>/plugin.lua
)