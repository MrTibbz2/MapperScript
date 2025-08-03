// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
// /include/utils/utils.hpp
#pragma once

#include <format>
#include <iostream>


class utils
{
public:
    template<typename... Args>
    static void print(std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...);
    }
    enum class LOG_TYPE
    {
        IS_WARNING = true,
        IS_INFO = false

    };
    enum class LOG_SOURCE {
        PLUGIN_MANAGER,
        SCRIPT_MANAGER,
        MAIN,
        UI_MANAGER,
        UNKNOWN
    };
    static inline std::string getLogSourceTag(const LOG_SOURCE source) {
        switch (source) {
        case LOG_SOURCE::PLUGIN_MANAGER:
            return "\033[1;34m[PLUGIN_MANAGER]\033[0m";  // Blue
        case LOG_SOURCE::SCRIPT_MANAGER:
            return "\033[1;32m[SCRIPT_ENGINE]\033[0m";  // Green
        case LOG_SOURCE::MAIN:
            return "\033[1;35m[BACKEND]\033[0m";        // Magenta
        case LOG_SOURCE::UI_MANAGER:
            return "\033[1;36m[FRONTEND]\033[0m";       // Cyan
        case LOG_SOURCE::UNKNOWN:
            return "\033[1;31m[SYSTEM]\033[0m";         // Red
        default:
            return "\033[1;30m[UNKNOWN]\033[0m";        // Gray
        }
    }
    static inline void log(const LOG_SOURCE source, const std::string& message, const bool isWarning = false) {
        std::string colorPrefix = isWarning ? "\033[1;31m" : "";  // Red if warning
        std::string colorSuffix = isWarning ? "\033[0m"    : "";

        utils::print("{} {}{}{}\n",
            getLogSourceTag(source),
            colorPrefix,
            message,
            colorSuffix
        );
    }

};
