/// math_consumer.cpp - Example dependent plugin demonstrating inter-plugin communication
/// 
/// This plugin shows how to:
/// 1. Depend on another plugin (test_plugin)
/// 2. Create C++ wrapper functions that call dependency functions via Lua
/// 3. Expose higher-level functionality built on dependencies

#include <iostream>
#include "plugins/PluginManager.h"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Global context pointer for dependency calls
static PluginManager::pluginContext* g_ctx = nullptr;

// Dependency wrapper functions - call test_plugin via Lua
int dependency_add(int a, int b) {
    if (!g_ctx) return 0;
    return g_ctx->call_lua<int>("test_plugin.add", a, b);
}

int dependency_multiply(int a, int b) {
    if (!g_ctx) return 0;
    return g_ctx->call_lua<int>("test_plugin.multiply", a, b);
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
        
        // Store context for dependency calls
        g_ctx = &ctx;
        
        // Test dependency is available
        try {
            int test_result = dependency_add(2, 3);
            std::cout << "[math_consumer] Dependency test: 2 + 3 = " << test_result << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[math_consumer] Failed to call dependency: " << e.what() << "\n";
            return false;
        }
        
        // Bind this plugin's functions to Lua namespace
        ctx.bind_function_namespace("math_consumer", "power_of_two", &cpp_power_of_two);
        ctx.bind_function_namespace("math_consumer", "sum_of_squares", &cpp_sum_of_squares);
        
        std::cout << "[math_consumer] Plugin loaded successfully\n";
        return true;
    }

    /// Plugin cleanup
    PLUGIN_EXPORT bool pluginShutdown(PluginManager::pluginContext& ctx) {
        std::cout << "[math_consumer] Shutting down plugin...\n";
        g_ctx = nullptr;
        return true;
    }
}