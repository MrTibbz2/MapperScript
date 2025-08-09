#include <iostream>
#include "include/plugins/PluginManager.h"
#include "include/Scripting/ScriptManager.h"

int main() {
    std::cout << "Testing namespace collision fix...\n";
    
    ScriptManager sm;
    if (sm.init() != ScriptManager::SMInitResult::SUCCESS) {
        std::cerr << "Failed to initialize ScriptManager\n";
        return 1;
    }
    
    PluginManager pm;
    pm.loadPluginsFromDir("plugins", sm);
    
    // Test if test_plugin.cpp_add is accessible
    try {
        auto result = sm.lua_state_mutable()["test_plugin"]["cpp_add"](2, 3);
        std::cout << "SUCCESS: test_plugin.cpp_add(2, 3) = " << result.get<int>() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: test_plugin.cpp_add not accessible: " << e.what() << "\n";
        return 1;
    }
    
    // Test if test_plugin.add (Lua function) is accessible
    try {
        auto result = sm.lua_state_mutable()["test_plugin"]["add"](5, 7);
        std::cout << "SUCCESS: test_plugin.add(5, 7) = " << result.get<int>() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "FAILED: test_plugin.add not accessible: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "All tests passed! Namespace collision fixed.\n";
    return 0;
}