



#include "ScriptManager.h"
#include <iostream>
#include <format>
#include <string_view>



ScriptManager::SMInitResult ScriptManager::init()
{
    try
    {
        lua_.open_libraries(sol::lib::base); // Load basic Lua libraries
    } catch (const std::exception& e)
    {
        std::cout << "init failed: " << e.what() << std::endl;
        return SMInitResult::FAILURE;
    }
    return SMInitResult::SUCCESS;
};

// Loads a Lua script from the given path and keeps it ready to run
ScriptManager::SMLoadResult ScriptManager::load_script(const std::filesystem::path& path)
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
std::thread ScriptManager::run_script(const std::filesystem::path& path)
{


};

// Saves loaded script paths to disk so they can be restored later
bool ScriptManager::save_loaded_scripts(const std::filesystem::path& json_out_path) const//idk what the end const does, jetbrains thought it'd be cool
{


};

// Loads previously saved script paths and loads them into memory
bool ScriptManager::restore_scripts_from_json(const std::filesystem::path& json_in_path)
{

};

// Updates watched files and reloads any that have changed
void ScriptManager::start_watcher_thread()
{
    std::cout << "Watcher Thread Started\n";
    std::thread([this]()
    {
        while (!HotReload_StopRequested)
        {
            for (const auto& [path, time] : file_watch_times_)
            {
                if (time != std::filesystem::last_write_time(path))
                {
                    std::cout << "Script at " << path << " has been modified. reloading...";
                    reload_script(path);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
        }
        std::cout << "Watcher Thread Stopped\n";
    }).detach();
};

void ScriptManager::stop_watcher_thread(){ HotReload_StopRequested = true; }

// Access to the Lua state for advanced usage if needed
const sol::state& ScriptManager::lua_state()
{
    return lua_;

};





// Internal helper to reload a single script
bool ScriptManager::reload_script(const std::filesystem::path& path) {
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
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Lua load exception in " << path << ": " << e.what() << "\n";
        return false;
    }
}


