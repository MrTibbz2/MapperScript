/// math_consumer.cpp - Example dependent plugin demonstrating inter-plugin communication
/// 
/// This plugin shows how to:
/// 1. Depend on another plugin (test_plugin)
/// 2. Create C++ wrapper functions that call dependency functions via Lua
/// 3. Expose higher-level functionality built on dependencies

#include <iostream>
#include "plugins/PluginManager.h"
#include "Scripting/ScriptManager.h"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Global ScriptManager pointer for dependency calls
static ScriptManager* g_sm = nullptr;
static sol::table test_plugin;

// Dependency wrapper functions - call test_plugin via Lua
int dependency_add(int a, int b) {
    if (!g_sm) return 0;

    sol::state_view& lua = g_sm->sol_state();
    sol::function require_fn = lua["require"];
    sol::table mod = require_fn("plugins.test_plugin.plugin");
    if (!mod.valid()) {
        std::cerr << "[math_consumer] ERROR: test_plugin module invalid\n";
        return 0;
    }

    const sol::object obj = mod["cpp_add"];
    if (!obj.is<sol::function>()) {
        std::cerr << "[math_consumer] ERROR: cpp_add is not a function\n";
        return 0;
    }

    const sol::function fn = obj.as<sol::function>();

    try {
        return fn(a, b);
    } catch (const std::exception& e) {
        std::cerr << "[math_consumer] Exception calling cpp_add: " << e.what() << "\n";
        return 0;
    }
}

int dependency_multiply(int a, int b) {
    std::cout << "[math_consumer] Dependency_multiply called..." << std::endl;
    if (!g_sm) return 0;

    sol::state_view& lua = g_sm->sol_state();
    sol::function require_fn = lua["require"];
    sol::table mod = require_fn("plugins.test_plugin.plugin");
    if (!mod.valid()) {
        std::cerr << "[math_consumer] ERROR: test_plugin module invalid\n";
        return 0;
    }

    const sol::object obj = mod["cpp_multiply"];
    if (!obj.is<sol::function>()) {
        std::cerr << "[math_consumer] ERROR: cpp_multiply is not a function\n";
        return 0;
    }

    const sol::function fn = obj.as<sol::function>();

    try {
        return fn(a, b);
    } catch (const std::exception& e) {
        std::cerr << "[math_consumer] Exception calling cpp_multiply: " << e.what() << "\n";
        return 0;
    }
}
// This plugin's own C++ functions
int cpp_power_of_two(int base) {
    std::cout << "[math_consumer] Computing " << base << "^2 using dependency functions\n";
    return dependency_multiply(base, base);
}

int cpp_sum_of_squares(int a, int b) {
    std::cout << "[math_consumer] Computing " << a << "^2 + " << b << "^2\n";
    int a_squared = dependency_multiply(a, a);
    int b_squared = dependency_multiply(b, b);
    return dependency_add(a_squared, b_squared);
}

extern "C" {
    /// Plugin initialization - setup dependency wrappers and bind own functions
    PLUGIN_EXPORT bool pluginLoad(PluginManager::pluginContext& ctx) {
        std::cout << "[math_consumer] Loading plugin with test_plugin dependency...\n";
        
        // Store ScriptManager for dependency calls
        g_sm = ctx.sm_;

        std::string current_path = ctx.sm_->sol_state()["package"]["path"];
        std::string plugin_path = "./?.lua;";

        if (current_path.find(plugin_path) == std::string::npos) {
            ctx.sm_->sol_state()["package"]["path"] = plugin_path + current_path;
        }

        // Require the Lua module
        test_plugin = ctx.sm_->sol_state()["require"]("plugins.test_plugin.plugin");
        if (!test_plugin.valid()) {
            std::cerr << "[math_consumer] Failed to load test_plugin module\n";
            return false;
        }

        
        // Bind this plugin's functions to Lua namespace
        ctx.bind_function_namespace("math_consumer", "power_of_two", &cpp_power_of_two);
        ctx.bind_function_namespace("math_consumer", "sum_of_squares", &cpp_sum_of_squares);
        
        std::cout << "[math_consumer] Plugin loaded successfully\n";
        //std::cout << dependency_multiply(2, 5) << std::endl;
        return true;
    }

    /// Plugin cleanup
    PLUGIN_EXPORT bool pluginShutdown(PluginManager::pluginContext& ctx) {
        std::cout << "[math_consumer] Shutting down plugin...\n";
        g_sm = nullptr;
        return true;
    }
}