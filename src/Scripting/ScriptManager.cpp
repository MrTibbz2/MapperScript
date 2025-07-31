



#include "ScriptManager.h"



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
bool ScriptManager::load_script(const fs::path& path)
{
    try
    {
        sol::load_result script = lua_.load_file(path.string());
    } catch (const std::exception& e)
    {
        std::cout << "load_script failed: " << e.what() << std::endl;
        return false;
    }
    return true;
};

// Executes a loaded script by its path (only one runs at a time)
bool ScriptManager::run_script(const fs::path& path)
{


};

// Saves loaded script paths to disk so they can be restored later
bool ScriptManager::save_loaded_scripts(const fs::path& json_out_path) const
{


};

// Loads previously saved script paths and loads them into memory
bool ScriptManager::restore_scripts_from_json(const fs::path& json_in_path)
{


};

// Updates watched files and reloads any that have changed
void ScriptManager::poll_file_watchers()
{


};

// Access to the Lua state for advanced usage if needed
sol::state& ScriptManager::lua_state()
{


};





// Internal helper to reload a single script
bool ScriptManager::reload_script(const fs::path& path)
{

};


