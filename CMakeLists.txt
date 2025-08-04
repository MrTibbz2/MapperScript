# Copyright (c) 2025 Lachlan McKenna
# All rights reserved. No part of this code may be used, copied, or distributed without permission.

cmake_minimum_required(VERSION 3.15)
project(MapperScript LANGUAGES CXX)

# Platform-specific flags
option(PLATFORM_LINUX "Set if building for Linux" OFF)
option(PLATFORM_WINDOWS "Set if building for Windows" OFF)
option(PLATFORM_OSX "set if building on osx" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Detected Linux OS")
    set(PLATFORM_LINUX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Detected Windows OS")
    set(PLATFORM_WINDOWS ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "ERROR: MAC DETECTED. INSTALLING MALWARE...")
    set(PLATFORM_OSX ON)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Source files ---
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

# --- Include directories ---
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendored/lua/include
        ${CMAKE_SOURCE_DIR}/vendored/linux_lua/include
        ${CMAKE_SOURCE_DIR}/vendored/osx_lua/include
        ${CMAKE_SOURCE_DIR}/vendored/sol2/include
        ${CMAKE_SOURCE_DIR}/vendored/dynalo/include
)

# --- Link library directory ---
if(PLATFORM_WINDOWS EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/lua)
elseif(PLATFORM_LINUX EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/linux_lua)
elseif(PLATFORM_OSX EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/osx_lua)
endif()

# --- Executable ---
add_executable(${PROJECT_NAME} ${SRC_FILES})

# --- Link Lua 5.4 dynamically ---
if(PLATFORM_LINUX)
    # Link to shared Lua library  
    target_link_libraries(${PROJECT_NAME} PRIVATE lua54 dl)

elseif(PLATFORM_OSX)
    # Link to shared Lua library
    message(STATUS "ARCHIE IF THIS FAILS ITS BECAUSE YOU DIDNT BLOODY LINK LUA MAN.")
    target_link_libraries(${PROJECT_NAME} PRIVATE lua)
endif()
if(PLATFORM_WINDOWS)
    # Link the import library explicitly by full path, no -l flags needed
    target_link_libraries(${PROJECT_NAME} PRIVATE
            "${CMAKE_SOURCE_DIR}/vendored/lua/liblua54.dll.a"
    )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts" "${CMAKE_CURRENT_BINARY_DIR}/scripts")
# --- Sol2 compile flags ---
target_compile_definitions(${PROJECT_NAME} PRIVATE SOL_ALL_SAFETIES_ON=1)

add_subdirectory(plugins/test_plugin)