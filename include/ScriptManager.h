// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.



#pragma once

#include <expected>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp> // For JSON config serialization
#include <optional>

namespace fs = std::filesystem;
using json = nlohmann::json;

class ScriptManager {
public:
    enum class SMInitResult
    {
        SUCCESS,
        FAILURE// just in case we need fail types for error handling, for now idfk
    };

    SMInitResult init();


    // Loads a Lua script from the given path and keeps it ready to run
    bool load_script(const fs::path& path);

    // Executes a loaded script by its path (only one runs at a time)
    bool run_script(const fs::path& path);

    // Saves loaded script paths to disk so they can be restored later
    bool save_loaded_scripts(const fs::path& json_out_path) const;

    // Loads previously saved script paths and loads them into memory
    bool restore_scripts_from_json(const fs::path& json_in_path);

    // Updates watched files and reloads any that have changed
    void poll_file_watchers();

    // Access to the Lua state for advanced usage if needed
    sol::state& lua_state();


private:


    // Internal helper to reload a single script
    bool reload_script(const fs::path& path);


    bool Exec_running = false;
    sol::state lua_; // The main Lua state
    std::unique_ptr<std::unordered_map<fs::path, sol::load_result>> loaded_scripts_; // Loaded script cache
    std::unordered_map<fs::path, std::filesystem::file_time_type> file_watch_times_; // Hot reload tracking
};


