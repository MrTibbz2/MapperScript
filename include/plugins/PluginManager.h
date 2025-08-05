// Copyright (c) 2025 Lachlan McKenna - MapperEngine
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#pragma once


#pragma once

#include <filesystem>
#include <string>
//#include <unordered_map>
//#include <iostream>
//#include <fstream>
#include "Scripting/ScriptManager.h"
#include <utility>
#include <dynalo.hpp>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

inline std::string getPlatformLibraryExtension() {
#ifdef _WIN32
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#elif defined(__linux__)
    return ".so";
#else
    return "";
#endif
}
/// PluginManager
/// - Loads plugins from disk, reads manifest.
/// - Loads shared libraries (dynalo) per platform.
/// - Calls plugin init function: MapperPluginInit(PluginContext&).
/// - Provides API lookup for plugins via getPluginAPI(name).
/// - Passes pointer to PluginManager in PluginContext for inter-plugin calls.
///
/// Plugin
/// - Holds plugin info: name, paths, loaded library handle.
/// - Owns plugin API instance
///
/// PluginContext
/// - Passed to plugins on init.
/// - Contains sol::state_view lua, PluginManager* for communication.
///
/// TODO: Inter-plugin calls
/// - Plugins query PluginManager for APIs of other plugins.
/// - Plugins use Lua or direct C++ interface to interact.

class PluginManager
{
public:
    struct ExportedPluginFunction // function exported by plugin
    {
        std::string name;
        std::any function;
    };
    struct pluginContext {
    private:
        // Hide the ScriptManager pointer so plugins can't call it directly
        ScriptManager* sm_ = nullptr;

    public:
        // Initialize with ScriptManager pointer (or ref converted to ptr)
        explicit pluginContext(ScriptManager& sm) : sm_(&sm) {}

        // Templated bind function that forwards only binding calls
        template<typename Func>
        void bind_function(const std::string& name, Func&& func)
        {

            sm_->bind_function(name, std::forward<Func>(func));
        }

        // Bind with namespace
        template<typename Func>
        void bind_function_namespace(const std::string& ns, const std::string& name, Func&& func) {
            sm_->bind_function_namespace(ns, name, std::forward<Func>(func));
        }


        // No public way to get sm_ pointer/reference, so no full access
    };
    enum class PLUGIN_INIT_FAILURE
    {
        FAILURE
    };
    struct RequiredPluginAPI // api required by every plugin, every plugin must have this
    {

        // pluginLoad: plugins export their exports and setup their context with the deps of other plugins

        std::pair<std::string, std::function<std::expected<std::vector<ExportedPluginFunction>, PLUGIN_INIT_FAILURE>(pluginContext&)>> pluginLoad = { "pluginLoad", nullptr };

        // PLUGIN INIT HAS BEEN DEPRECATED.

        // // pluginInit: plugins should access other functions exported by plugins if they have dependencies.
        // // plugins may init anything else + bind functions using context.
        //std::pair<std::string, pluginFunc> pluginInit = { "pluginShutdown", nullptr };
        std::pair<std::string, std::function<int(pluginContext&)>> pluginShutdown = { "pluginShutdown", nullptr };
    };


    struct plugin {
        std::string name;
        std::string description;
        std::string version;
        fs::path folder_path;
        fs::path lib_path;
        fs::path luaScript_path;
        std::optional<dynalo::library> lib;
        RequiredPluginAPI RequiredAPI;
        std::vector<ExportedPluginFunction> exportedFunctions;


        plugin() = delete; // allow default construction

        explicit plugin(const fs::path& folder)
            : name(folder.filename().string()),
              description(""),
              version(""),
              folder_path(folder),
              lib_path(""),
              luaScript_path(""),
              lib(std::nullopt) // 👈 initialize dynalo::library (empty handle)
        {}
    };



    bool loadPlugin(const fs::path& pluginDir, ScriptManager& sm) const;

    void loadPluginsFromDir(const fs::path& plugin_dir, ScriptManager& sm) const
    {
        if (!fs::exists(plugin_dir) || !fs::is_directory(plugin_dir)) {
            std::cerr << "[PluginLoader] Plugin directory not found: " << plugin_dir << "\n";
            return;
        }

        for (const auto& entry : fs::directory_iterator(plugin_dir)) {
            if (!entry.is_directory())
                continue;

            const fs::path& folder = entry.path();
            try {
                loadPlugin(folder, sm);
                std::cout << "[PluginLoader] Loaded plugin from " << folder << "\n";
            } catch (const std::exception& e) {
                std::cerr << "[PluginLoader] Failed to load plugin in " << folder << ": " << e.what() << "\n";
            }
        }
    }


    [[nodiscard]] bool CheckIfPluginExists(const std::string& name) const
    {
        if (!loadedPlugins) {
            // Handle error or just return false
            return false;
        }

        for (const plugin& ExistingPlugin : *loadedPlugins) {
            if (ExistingPlugin.name == name)
                return true;
        }
        return false;

    }




private:
    using pluginVector = std::vector<plugin>;
    std::unique_ptr<pluginVector> loadedPlugins = std::make_unique<pluginVector>();
};