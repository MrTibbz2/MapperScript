
/// test_plugin.cpp - Example plugin demonstrating Lua-based inter-plugin communication
/// 
/// This plugin:
/// 1. Binds C++ functions to "test_plugin" namespace in Lua
/// 2. Exposes public API through plugin.lua header
/// 3. Shows how to create dependency wrapper functions

#include <iostream>
#include "plugins/PluginManager.h"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Private C++ functions - only accessible through Lua bindings
int cpp_add_two_numbers(int a, int b) { 
    std::cout << "[test_plugin] C++ add: " << a << " + " << b << " = " << (a + b) << "\n";
    return a + b; 
}

int cpp_multiply_two_numbers(int a, int b) { 
    std::cout << "[test_plugin] C++ multiply: " << a << " * " << b << " = " << (a * b) << "\n";
    return a * b; 
}

// Example dependency wrapper function (if this plugin had dependencies)
// int dependency_some_function(int x) {
//     return ctx.call_lua<int>("other_plugin.some_function", x);
// }

extern "C" {
    /// Plugin initialization - bind functions to Lua namespace
    PLUGIN_EXPORT bool pluginLoad(PluginManager::pluginContext& ctx) {
        std::cout << "[test_plugin] Loading plugin...\n";
        
        // Bind C++ functions to test_plugin namespace in Lua
        std::cout << "[test_plugin] Binding test_plugin.cpp_add\n";
        ctx.bind_function_namespace("test_plugin", "cpp_add", &cpp_add_two_numbers);
        std::cout << "[test_plugin] Binding test_plugin.cpp_multiply\n";
        ctx.bind_function_namespace("test_plugin", "cpp_multiply", &cpp_multiply_two_numbers);
        
        // Also bind to global namespace for backward compatibility
        ctx.bind_function("cpp_add_two_numbers", &cpp_add_two_numbers);
        
        std::cout << "[test_plugin] Functions bound to Lua successfully\n";
        return true; // Success
    }

    /// Plugin cleanup
    PLUGIN_EXPORT bool pluginShutdown(PluginManager::pluginContext& ctx) {
        std::cout << "[test_plugin] Shutting down plugin...\n";
        // Clean up plugin resources here if needed
        return true; // Success
    }
} // extern "C"
