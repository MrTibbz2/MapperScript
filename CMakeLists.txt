cmake_minimum_required(VERSION 3.15)
project(MapperScript LANGUAGES CXX)

# Platform-specific flags
option(PLATFORM_LINUX "Set if building for Linux" OFF)
option(PLATFORM_WINDOWS "Set if building for Windows" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Detected Linux OS")
    set(PLATFORM_LINUX ON)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Detected Windows OS")
    set(PLATFORM_WINDOWS ON)
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
        ${CMAKE_SOURCE_DIR}/vendored/sol2/include
)

# --- Link library directory ---
if(PLATFORM_WINDOWS EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/lua)
elseif(PLATFORM_LINUX EQUAL ON)
    link_directories(${CMAKE_SOURCE_DIR}/vendored/linux_lua)
endif()

# --- Executable ---
add_executable(${PROJECT_NAME} ${SRC_FILES})

# --- Link Lua 5.4 ---
if(PLATFORM_LINUX)
    # Link directly to the static library (without using -l flag)
    target_link_libraries(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/vendored/linux_lua/liblua54.a")
elseif(PLATFORM_WINDOWS)
    target_link_libraries(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/vendored/lua/liblua54.a")
endif()

# --- Copy DLL for Windows builds (if dynamic) ---
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/vendored/lua/lua54.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# --- Sol2 compile flags ---
target_compile_definitions(${PROJECT_NAME} PRIVATE SOL_ALL_SAFETIES_ON=1)
