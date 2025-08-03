// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
// /src/scripting/scriptManager.cpp
#include "Scripting/ScriptManager.h"

#include <chrono>
#include <iostream>
#include <format>
#include <string_view>
#include <fstream> // For std::ifstream and std::ofstream
#include <iomanip> // For std::setw
#include <functional> // For std::ref
#include <nlohmann/json.hpp> // For JSON config serialization
#include <future> // For std::async and std::future

namespace fs = std::filesystem; // Alias for std::filesystem
std::string FileTimeTypeToString(const std::filesystem::file_time_type& ftime) {
    // Convert file_time_type to system_clock::time_point (C++20)
    auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);

    // Convert to time_t for formatting
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    // Convert to tm structure
    std::tm* timeinfo = std::localtime(&cftime);

    // Format time to string
    std::ostringstream oss;
    if (timeinfo) {
        oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    } else {
        return "Invalid Time";
    }
}


ScriptManager::SMInitResult ScriptManager::init()
{
    try
    {
        lua_.open_libraries(sol::lib::base, sol::lib::package); // Load basic Lua libraries
    } catch (const std::exception& e)
    {
        std::cout << "init failed: " << e.what() << std::endl;
        return SMInitResult::FAILURE;
    }
    return SMInitResult::SUCCESS;
};

// Loads a Lua script from the given path and keeps it ready to run
ScriptManager::SMLoadResult ScriptManager::load_script(const fs::path& path)
{
    // Check early if script is already loaded
    if (loaded_scripts_.contains(path)) {
        return SMLoadResult::FILE_ALREADY_LOADED;  // Already loaded
    }

    try {
        sol::load_result script = lua_.load_file(path.string());

        // Check for load errors
        if (!script.valid()) {
            const sol::error err = script;
            std::cerr << "Lua load error in " << path << ": " << err.what() << "\n";
            return SMLoadResult::FILE_LOAD_ERROR;
        }

        // we take the path of the script, and the script, and shove it up the ass of the class
        loaded_scripts_.emplace(path, std::move(script));

        try {
            auto last_write = std::filesystem::last_write_time(path);
            file_watch_times_.insert({path, last_write});
            std::cout << "last write at: " << last_write;
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error for " << path << ": " << e.what() << "\n";
            return SMLoadResult::FILE_LOAD_ERROR;
        }

        // note the watch time, if this changes we reload, because it has been modified.


        return SMLoadResult::FILE_LOAD_SUCCESS;

    } catch (const std::exception& e) {
        std::cerr << "Exception loading script from " << path << ": " << e.what() << "\n";
        return SMLoadResult::TS_PMO; // that means there's an error loading that idfk what happened
    }
}


// Executes a loaded script by its path (only one runs at a time)
std::future<void> ScriptManager::run_script(const fs::path& path)
{
    if (Exec_running) {
        std::cerr << "A script is already running. Only one script can run at a time.\n";
        return std::future<void>(); // Return an invalid future
    }

    auto it = loaded_scripts_.find(path);
    if (it == loaded_scripts_.end()) {
        std::cerr << "Script not loaded: " << path << "\n";
        return std::future<void>(); // Return an invalid future
    }

    Exec_running = true;
    // Capture the sol::load_result by value to ensure it's valid in the new thread
    return std::async(std::launch::async, [this, script_to_run = std::move(it->second)]() mutable {
        try {
            // Execute the loaded script
            sol::protected_function_result result = script_to_run();
            if (!result.valid()) {
                sol::error err = result;
                std::cerr << "Lua script execution error: " << err.what() << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception during script execution: " << e.what() << "\n";
        }
        Exec_running = false;
    });
}

// Saves loaded script paths to disk so they can be restored later
bool ScriptManager::save_loaded_scripts(const fs::path& json_out_path) const
{
    nlohmann::json j;

    for (const auto& pair : loaded_scripts_) {
        j["scripts"].push_back(pair.first.string());
    }

    try {
        std::ofstream o(json_out_path);
        if (!o.is_open()) {
            std::cerr << "Error opening file for writing: " << json_out_path << "\n";
            return false;
        }
        o << std::setw(4) << j << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving loaded scripts to " << json_out_path << ": " << e.what() << "\n";
        return false;
    }
}

// Loads previously saved script paths and loads them into memory
bool ScriptManager::restore_scripts_from_json(const fs::path& json_in_path)
{
    try {
        std::ifstream i(json_in_path);
        if (!i.is_open()) {
            std::cerr << "Error opening JSON file: " << json_in_path << "\n";
            return false;
        }
        nlohmann::json j;
        i >> j;

        if (j.contains("scripts") && j["scripts"].is_array()) {
            bool all_successful = true;
            for (const auto& script_path_str : j["scripts"]) {
                if (script_path_str.is_string()) {
                    fs::path script_path = script_path_str.get<std::string>();
                    SMLoadResult result = load_script(script_path);
                    if (result != SMLoadResult::FILE_LOAD_SUCCESS && result != SMLoadResult::FILE_ALREADY_LOADED) {
                        std::cerr << "Failed to load script from JSON: " << script_path << "\n";
                        all_successful = false;
                    }
                }
            }
            return all_successful;
        } else {
            std::cerr << "JSON file does not contain a 'scripts' array: " << json_in_path << "\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error restoring scripts from " << json_in_path << ": " << e.what() << "\n";
        return false;
    }
}

// Updates watched files and reloads any that have changed
void ScriptManager::start_watcher_thread()
{
    std::cout << "Watcher Thread Started\n";
    std::thread([this]()
    {
        while (!HotReload_StopRequested)
        {
            std::vector<fs::path> changed_scripts;

            // First pass: check which files changed
            for (const auto& [path, time] : file_watch_times_)
            {
                try {
                    auto current_time = std::filesystem::last_write_time(path);
                    if (time != current_time)
                    {
                        changed_scripts.push_back(path);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error checking file time for " << path << ": " << e.what() << "\n";
                }
            }

            // Second pass: reload changed files and update watch times
            for (const auto& path : changed_scripts)
            {
                std::cout << "Script at " << path << " has been modified. Reloading...\n";
                if (reload_script(path))
                {
                    file_watch_times_[path] = std::filesystem::last_write_time(path);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(750));
        }

        std::cout << "Watcher Thread Stopped\n";
    }).detach();
}

void ScriptManager::stop_watcher_thread(){ HotReload_StopRequested = true; }

// Access to the Lua state for advanced usage if needed
const sol::state& ScriptManager::lua_state()
{
    return lua_;

};

void ScriptManager::bind_function(const std::string& name, std::function<int(int, int)> func)
{
    lua_.set_function(name, func);
}





// Internal helper to reload a single script
bool ScriptManager::reload_script(const fs::path& path) {
    if (!loaded_scripts_.contains(path)) {
        return false;  // Can't reload something that's not loaded
    }

    try {
        sol::load_result script = lua_.load_file(path.string());

        if (!script.valid()) {
            const sol::error err = script;
            std::cout << "Script reload error at " << path << ": " << err.what();
            return false;
        }

        loaded_scripts_.erase(path);
        loaded_scripts_.emplace(path, std::move(script));

        // file_watch_times_.erase(path);
        // file_watch_times_.emplace(path, std::filesystem::last_write_time(path)); causes segfault.

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Lua load exception in " << path << ": " << e.what() << "\n";
        return false;
    }
}