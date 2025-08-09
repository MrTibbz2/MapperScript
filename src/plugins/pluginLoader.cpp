// Copyright (c) 2025 Lachlan McKenna - MapperEngine
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
// /src/plugins/pluginLoader.h

#include "plugins/PluginManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;
using pluginFunc = std::function<int(PluginManager::pluginContext&)>;


const std::string libName =
#ifdef _WIN32
        "plugin.dll";
#elif __APPLE__
        "plugin.dylib";
#else
        "plugin.so";
#endif




bool PluginManager::loadPluginMetadata(const fs::path& pluginDir) const
{

    // Basic sanity checks
    if (!fs::exists(pluginDir) || !fs::is_directory(pluginDir)) {
        std::cerr << "Plugin directory does not exist or is not a directory: " << pluginDir << '\n';
        return false;
    }

    // Determine the library filename based on platform
    const std::string libName =
#ifdef _WIN32
        "plugin.dll";
#elif __APPLE__
        "plugin.dylib";
#else
        "plugin.so";
#endif

    const fs::path libPath = pluginDir / libName;
    const fs::path luaPath = pluginDir / "plugin.lua";

    // Check library file exists before trying to load
    if (!fs::exists(libPath)) {
        std::cerr << "Plugin library not found: " << libPath << '\n';
        return false;
    }

    // Check Lua script file existence (warn only, optional)
    if (!fs::exists(luaPath)) {
        std::cerr << "Warning: plugin.lua script not found in plugin directory: " << luaPath << '\n';

    }
    plugin newPlugin(pluginDir);
    newPlugin.lib_path = libPath;
    newPlugin.luaScript_path = luaPath;

    fs::path json_input_path = pluginDir / "metadata.json";

    auto setDefaultPlugin = [&]() {
        newPlugin.name = pluginDir.filename().string();
        newPlugin.description = "No description";
        newPlugin.version = "0.1";
        std::cout << "Using default plugin metadata\n";
    };

    if (!fs::exists(json_input_path)) {
        std::cout << "metadata.json not found\n";
        setDefaultPlugin();
    } else {
        std::ifstream input_file(json_input_path);
        if (!input_file) {
            std::cerr << "Failed to open file: " << json_input_path << "\n";
            setDefaultPlugin();
        } else {
            json metadata;
            try {
                input_file >> metadata;
            } catch (const json::parse_error& e) {
                std::cerr << "Failed to parse JSON: " << e.what() << '\n';
                setDefaultPlugin();
            }

            newPlugin.name = metadata.value("name", pluginDir.filename().string());
            newPlugin.version = metadata.value("version", "0.0.0");
            newPlugin.description = metadata.value("description", "N/A");
            newPlugin.dependencies = metadata.value("dependencies", json::array());

            if (CheckIfPluginExists(newPlugin.name)) {
                std::cout << "Plugin named \"" << newPlugin.name << "\" already exists, discarding...\n";
                return false;
            }
        }
    }
    // Construct plugin with folder path (other paths will be set below)

    loadedPlugins->push_back(std::move(newPlugin));
    return true;
}


bool PluginManager::loadPluginLibrary(const plugin& newPlugin, ScriptManager& sm)
{
    
    return true;
}
