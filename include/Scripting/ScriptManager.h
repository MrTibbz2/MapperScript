// Copyright (c) 2025 Lachlan McKenna - MapperEngine
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
#include <future>
#include <nlohmann/json.hpp>
#include <optional>
#include <thread>
#include <chrono>


using json = nlohmann::json;

class ScriptManager {
public:
    enum class SMInitResult
    {
        SUCCESS,
        FAILURE// just in case we need fail types for error handling, for now idfk
    };
    enum class SMLoadResult
    {
        FILE_LOAD_ERROR,
        FILE_LOAD_SUCCESS,
        FILE_ALREADY_LOADED,
        TS_PMO

    };


    SMInitResult init();


    // Loads a Lua script from the given path and keeps it ready to run
    SMLoadResult load_script(const std::filesystem::path& path);

    // Executes a loaded script by its path (only one runs at a time)
    std::future<void> run_script(const std::filesystem::path& path);

    // Saves loaded script paths to disk so they can be restored later
    bool save_loaded_scripts(const std::filesystem::path& json_out_path = "scripts.json") const;

    // Loads previously saved script paths and loads them into memory
    bool restore_scripts_from_json(const std::filesystem::path& json_in_path = "scripts.json");

    // Updates watched files and reloads any that have changed
    void start_watcher_thread();

    void stop_watcher_thread();

    // Access to the Lua state for advanced usage if needed
    const sol::state& lua_state();

    template<typename Func>
    void bind_function(const std::string& name, Func&& func) {
        // Simple direct registration
        std::cout << "[pluginMGR] Binding function: " << name << '\n';
        lua_.set_function(name, std::forward<Func>(func));
    }

    // Bind function into a Lua namespace table
    template<typename Func>
    void bind_function_namespace(const std::string& ns, const std::string& name, Func&& func) {
        sol::table table = lua_[ns];
        if (!table.valid()) {
            table = lua_.create_table(ns);
        }
        table.set_function(name, std::forward<Func>(func));
    }


    void print_fileTimes() const
    {
        std::cout << "file count: " << file_watch_times_.size() << "\n";
        std::cout << "times: " << file_watch_times_.begin()->first << "\n";
        std::cout << "paths: " << file_watch_times_.begin()->second << "\n";

    }

private:


    // Internal helper to reload a single script
    bool reload_script(const std::filesystem::path& path);

    std::atomic_bool HotReload_StopRequested = false;
    bool Exec_running = false;
    sol::state lua_; // The main Lua state



    std::unordered_map<std::filesystem::path, sol::load_result> loaded_scripts_; // Loaded script cache
    std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> file_watch_times_; // Hot reload tracking
};


