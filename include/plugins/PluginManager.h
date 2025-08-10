// Copyright (c) 2025 Lachlan McKenna - MapperEngine
// All rights reserved. No part of this code may be used, copied, or distributed without permission.

#pragma once


#pragma once

#include <filesystem>
#include <string>
#include <algorithm>
#include "Scripting/ScriptManager.h"
#include <utility>
#include <dynalo.hpp>

#include <nlohmann/json.hpp>

#include "nlohmann/json.hpp"

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
/// PluginManager - Simplified Lua-based Inter-Plugin Communication
/// 
/// Plugins communicate through Lua headers (plugin.lua files) that act as public APIs.
/// Each plugin:
/// 1. Binds C++ functions to Lua namespaces in pluginLoad()
/// 2. Exposes public API through plugin.lua header
/// 3. Calls dependencies via ctx.call_lua() wrapper functions
/// 
/// Benefits:
/// - Clean separation of public/private functions
/// - Type-safe inter-plugin calls through C++ wrappers
/// - Easy debugging through Lua call tracing
/// - Minimal boilerplate code
///
/// Example Usage:
/// Plugin A (math_plugin):
///   - Binds: ctx.bind_function_namespace("math_plugin", "add", &cpp_add)
///   - plugin.lua: function math_plugin.add(a, b) return cpp_add(a, b) end
/// 
/// Plugin B (depends on A):
///   - Wrapper: int dep_add(int a, int b) { return ctx.call_lua<int>("math_plugin.add", a, b); }
///   - Usage: int result = dep_add(5, 3);  // Calls through Lua to Plugin A

class PluginManager
{
public:


    /// Plugin context passed to plugin init/shutdown functions
    /// Provides controlled access to Lua binding and inter-plugin calls
    struct pluginContext {



    public:
        ScriptManager* sm_ = nullptr;
        explicit pluginContext(ScriptManager& sm) : sm_(&sm) {}

        /// Bind C++ function to global Lua namespace
        template<typename Func>
        void bind_function(const std::string& name, Func&& func) {
            sm_->bind_function(name, std::forward<Func>(func));
        }

        /// Bind C++ function to specific Lua namespace (recommended for plugins)
        template<typename Func>
        void bind_function_namespace(const std::string& ns, const std::string& name, Func&& func) {
            std::cout << "[pluginContext] Binding " << ns << "." << name << "\n";
            sm_->bind_function_namespace(ns, name, std::forward<Func>(func));
        }


    };
    /// Required API functions that every plugin must implement
    struct RequiredPluginAPI {
        /// Plugin initialization - bind functions and setup dependencies
        /// Return true on success, false on failure
        std::pair<std::string, std::function<bool(pluginContext&)>> pluginLoad = { "pluginLoad", nullptr };
        
        /// Plugin cleanup - release resources
        /// Return true on success, false on failure
        std::pair<std::string, std::function<bool(pluginContext&)>> pluginShutdown = { "pluginShutdown", nullptr };
    };


    /// Plugin metadata and runtime information
    struct plugin {
        bool loaded = false;
        std::string name;
        std::string description;
        std::string version;
        json dependencies;
        fs::path folder_path;
        fs::path lib_path;
        fs::path luaScript_path;  // plugin.lua header file
        std::optional<dynalo::library> lib;
        RequiredPluginAPI RequiredAPI;


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
                loadPluginMetadata(folder);
                std::cout << "[PluginLoader] Loaded plugin from " << folder << "\n";
            } catch (const std::exception& e) {
                std::cerr << "[PluginLoader] Failed to load plugin in " << folder << ": " << e.what() << "\n";
            }

        }
        if (auto loadOrder = ResolveLoadOrder()) {
            std::cout << "[PluginLoader] Load order resolved, found " << loadOrder->size() << " plugins\n";

            for (plugin& entry : *loadOrder) {
                std::cout << "[PluginLoader] Loading: " << entry.name << "\n";
                
                // Debug: Check test_plugin namespace before loading math_consumer
                if (entry.name == "math_consumer") {
                    try {
                        sol::table test_ns = sm.sol_state()["test_plugin"];
                        if (test_ns.valid()) {
                            sol::function cpp_add = test_ns["cpp_add"];
                            std::cout << "[DEBUG] Before math_consumer load - test_plugin.cpp_add valid: " << (cpp_add.valid() ? "YES" : "NO") << "\n";
                            if (cpp_add.valid()) {
                                int result = cpp_add(10, 20);
                                std::cout << "[DEBUG] test_plugin.cpp_add(10, 20) = " << result << "\n";
                            }
                        } else {
                            std::cout << "[DEBUG] test_plugin namespace is INVALID\n";
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "[DEBUG] Exception checking namespace: " << e.what() << "\n";
                    }
                }
                
                if (loadPluginLibrary(entry, sm)) {
                    // plugin.lua files are loaded via require() in scripts, not directly executed
                    if (fs::exists(entry.luaScript_path)) {
                        std::cout << "[PluginLoader] Plugin Lua header available: " << entry.luaScript_path << "\n";
                    }
                    entry.loaded = true;
                    std::cout << "[PluginLoader] Successfully loaded: " << entry.name << "\n";
                } else {
                    std::cerr << "[PluginLoader] Failed to load: " << entry.name << "\n";
                }
            }
        } else {
            std::cerr << "[PluginLoader] Failed to resolve plugin load order: " << loadOrder.error() << "\n";
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
    bool loadPluginMetadata(const std::filesystem::path& pluginDir) const;
    static bool loadPluginLibrary(const plugin& newPlugin, ScriptManager& sm);

    [[nodiscard]]
    std::expected<std::reference_wrapper<plugin>, bool> GetPluginByName(const std::string& name) const
    {
        for (plugin& ExistingPlugin : *loadedPlugins)
        {
            if (ExistingPlugin.name == name)
                return std::ref(ExistingPlugin); // wrap the reference
        }
        return std::unexpected(false); // return an error value
    }
    static bool contains_plugin_with_name(const std::vector<std::reference_wrapper<plugin>>& plugins, const std::string& target) {
        return std::any_of(plugins.begin(), plugins.end(),
                           [&](const std::reference_wrapper<plugin>& p_ref) {
                               return p_ref.get().name == target;
                           });
    }
    [[nodiscard]]
    std::expected<std::vector<std::reference_wrapper<plugin>>, std::string>
    ResolveLoadOrder() const
    {
        if (!loadedPlugins) { return std::unexpected("loaded plugins does not exist.");}
        std::vector<std::reference_wrapper<plugin>> PluginLoadOrder;
        size_t lastSize = 0;
        
        while (loadedPlugins->size() != PluginLoadOrder.size())
        {
            bool progressMade = false;
            for (plugin& PluginMetadata : *loadedPlugins)
            {
                if (contains_plugin_with_name(PluginLoadOrder, PluginMetadata.name)) { continue; }
                
                if (!PluginMetadata.dependencies.is_array())
                {
                    std::cerr << "[PluginLoader] Dependencies format not supported for " << PluginMetadata.name << "\n";
                    continue;
                }
                
                bool allDepsResolved = true;
                for (const json& dep : PluginMetadata.dependencies)
                {
                    if (!dep.is_object()) continue;
                    std::string depName = dep.value("name", std::string());
                    if (!contains_plugin_with_name(PluginLoadOrder, depName))
                    {
                        allDepsResolved = false;
                        break;
                    }
                }
                
                if (allDepsResolved)
                {
                    PluginLoadOrder.emplace_back(PluginMetadata);
                    std::cout << "[PluginLoader] Added to load order: " << PluginMetadata.name << "\n";
                    progressMade = true;
                }
            }
            
            if (!progressMade) {
                return std::unexpected("Circular dependency or missing dependency detected");
            }
        }
        return PluginLoadOrder;
    }


private:
    using pluginVector = std::vector<plugin>;
    std::unique_ptr<pluginVector> loadedPlugins = std::make_unique<pluginVector>();
};