
// test_plugin.cpp
#include <iostream>

#include "plugins/PluginManager.h"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

int cpp_add_two_numbers(int a, int b) { return a + b; }


extern "C" {

    // Called when the plugin is loaded
    PLUGIN_EXPORT int pluginInit(PluginManager::pluginContext& ctx) {
        std::cout << "[test_plugin] pluginInit called\n";
        // Initialize plugin resources, hook into ctx as needed
        ctx.bindFunction("cpp_add_two_numbers", cpp_add_two_numbers);
        return 0; // success
    }

    // Called when the plugin is unloaded/shutdown
    PLUGIN_EXPORT int pluginShutdown(PluginManager::pluginContext& ctx) {
        std::cout << "[test_plugin] pluginShutdown called\n";
        // Clean up plugin resources here
        return 0; // success
    }

} // extern "C"
