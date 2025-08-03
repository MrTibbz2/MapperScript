


#include "plugins/PluginManager.h"


#include <iostream>
#include <filesystem>
#include <optional>
using pluginFunc = std::function<int(PluginManager::pluginContext&)>;

bool PluginManager::loadPlugin(const fs::path& pluginDir) {
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

    // Construct plugin with folder path (other paths will be set below)
    plugin newPlugin(pluginDir);
    newPlugin.lib_path = libPath;
    newPlugin.luaScript_path = luaPath;

    // Load the dynamic library, wrapped in std::optional to handle construction failure gracefully
    std::optional<dynalo::library> loadedLib;
    try {
        loadedLib.emplace(libPath.string());
    } catch (const std::exception& e) {
        std::cerr << "Failed to load plugin library at " << libPath << ": " << e.what() << '\n';
        return false;
    }
    newPlugin.lib = std::move(*loadedLib);

    // Helper lambda to load a function pointer from the library
    auto loadPluginFunction = [&](const char* funcName) -> std::optional<pluginFunc> {
        try {
            auto rawFunc = newPlugin.lib.get_function<int(pluginContext&)>(funcName);
            return pluginFunc{ [rawFunc](pluginContext& ctx) { return rawFunc(ctx); } };
        } catch (...) {
            return std::nullopt;
        }
    };

    // Load required pluginInit function
    auto initFuncOpt = loadPluginFunction("pluginInit");
    if (!initFuncOpt.has_value()) {
        std::cerr << "Missing required symbol: pluginInit\n";
        return false;
    }
    newPlugin.api.pluginInit.second = *initFuncOpt;

    // Load optional pluginShutdown function
    auto shutdownFuncOpt = loadPluginFunction("pluginShutdown");
    newPlugin.api.pluginShutdown.second = shutdownFuncOpt.value_or(nullptr);

    // Call pluginInit
    if (newPlugin.api.pluginInit.second) {
        int initResult = 0;
        try {
            pluginContext newCtx;
            initResult = newPlugin.api.pluginInit.second(newCtx);
        } catch (const std::exception& e) {
            std::cerr << "Exception thrown during pluginInit: " << e.what() << '\n';
            return false;
        } catch (...) {
            std::cerr << "Unknown exception thrown during pluginInit\n";
            return false;
        }

        if (initResult != 0) {
            std::cerr << "Plugin init failed with code " << initResult << '\n';
            return false;
        }
    }

    // TODO: load from Lua or JSON here later
    newPlugin.name = pluginDir.filename().string();
    newPlugin.description = "No description";
    newPlugin.version = "0.1";

    // Store loaded plugin
    loadedPlugins.push_back(std::move(newPlugin));
    return true;
}