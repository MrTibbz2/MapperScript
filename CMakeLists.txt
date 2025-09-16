# Copyright (c) 2025 Lachlan McKenna
# All rights reserved. No part of this code may be used, copied, or distributed without permission.
#CmakeLists.txt (root)
cmake_minimum_required(VERSION 3.15)
project(MapperScript LANGUAGES CXX)

# Platform-specific flags
message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
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
        ${CMAKE_SOURCE_DIR}/vendored/sol2/include
        ${CMAKE_SOURCE_DIR}/vendored/dynalo/include
        ${CMAKE_SOURCE_DIR}/vendored/jsonrpccxx
)

# Add platform-specific Lua includes if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/vendored/linux_lua/${CMAKE_SYSTEM_PROCESSOR}/include")
    include_directories(${CMAKE_SOURCE_DIR}/vendored/linux_lua/${CMAKE_SYSTEM_PROCESSOR}/include)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vendored/lua/include")
    include_directories(${CMAKE_SOURCE_DIR}/vendored/lua/include)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/vendored/osx_lua/include")
    include_directories(${CMAKE_SOURCE_DIR}/vendored/osx_lua/include)
endif()

# --- Link library directory ---
if(PLATFORM_WINDOWS EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/lua)
elseif(PLATFORM_LINUX)

    link_directories(${CMAKE_SOURCE_DIR}/vendored/linux_lua/${CMAKE_SYSTEM_PROCESSOR})
elseif(PLATFORM_OSX EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/osx_lua)
endif()

# --- Executable ---
add_executable(${PROJECT_NAME} ${SRC_FILES})

# --- Link Lua 5.4 dynamically ---
if(PLATFORM_LINUX)
    # Check if vendored Lua library exists for this architecture
    set(VENDORED_LUA_PATH "${CMAKE_SOURCE_DIR}/vendored/linux_lua/${CMAKE_SYSTEM_PROCESSOR}/liblua54.so")
    if(EXISTS "${VENDORED_LUA_PATH}")
        message(STATUS "Using vendored Lua library: ${VENDORED_LUA_PATH}")
        target_link_libraries(${PROJECT_NAME} PRIVATE "${VENDORED_LUA_PATH}" dl)
        # Set RPATH so executable can find the Lua library at runtime
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "${CMAKE_SOURCE_DIR}/vendored/linux_lua/${CMAKE_SYSTEM_PROCESSOR}"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    else()
        message(STATUS "Vendored Lua not found for ${CMAKE_SYSTEM_PROCESSOR}, using system Lua")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LUA REQUIRED lua5.4)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES} dl)
        target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIRS})
    endif()

elseif(PLATFORM_OSX)
    # Link to shared Lua library
    message(STATUS "ARCHIE IF THIS FAILS ITS BECAUSE YOU DIDNT BLOODY LINK LUA MAN.")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/vendored/osx_lua/liblua.dylib)
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
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SOL_ALL_SAFETIES_ON=0
    SOL_NO_EXCEPTIONS=0
    SOL_SAFE_USERTYPE=1
)

add_subdirectory(plugins/test_plugin)
add_subdirectory(plugins/math_consumer)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-deprecated-literal-operator)
endif()