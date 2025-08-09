# Plugin Communication System Refactor

## Overview

The plugin system has been refactored to use a simplified Lua-based inter-plugin communication approach. This eliminates complex C++ API management while providing clean, type-safe inter-plugin calls.

## Key Changes Made

### 1. Simplified Plugin API Returns
- **Before**: `std::expected<std::vector<ExportedPluginFunction>, PLUGIN_INIT_FAILURE>`
- **After**: `bool` (true = success, false = failure)

### 2. Removed Complex Structures
- Eliminated `ExportedPluginFunction` struct with `std::any`
- Removed `ExportedPluginAPI` struct
- Removed `PLUGIN_INIT_FAILURE` enum
- Simplified `RequiredPluginAPI` to use bool returns

### 3. Added Inter-Plugin Communication
- Added `call_lua<R, Args...>()` method to `pluginContext`
- Plugins can call other plugins via: `ctx.call_lua<int>("other_plugin.function", args...)`

### 4. Enhanced Documentation
- Added comprehensive comments explaining the new system
- Provided usage examples in code comments

## New Plugin Development Pattern

### 1. Plugin C++ Implementation
```cpp
// Bind functions to Lua namespace
PLUGIN_EXPORT bool pluginLoad(PluginManager::pluginContext& ctx) {
    ctx.bind_function_namespace("my_plugin", "my_function", &cpp_my_function);
    return true;
}
```

### 2. Plugin Lua Header (plugin.lua)
```lua
local Module = {}

function Module.public_function(a, b)
    return my_plugin.my_function(a, b)  -- Calls C++ function via global namespace
end

return Module
```

### 3. Dependent Plugin Usage
```cpp
// Create wrapper for dependency calls
int dependency_function(int a, int b) {
    return ctx.call_lua<int>("other_plugin.public_function", a, b);
}
```

## Critical Implementation Notes

### Namespace Collision Fix
- **Issue**: plugin.lua execution was overwriting C++ bound functions
- **Solution**: Use module pattern - plugin.lua returns tables, doesn't modify global namespaces
- **Key**: Don't test dependencies during pluginLoad() - test after all plugins loaded

## Benefits of New System

1. **Simplicity**: No complex C++ API management
2. **Type Safety**: Sol2 handles type conversions automatically
3. **Debugging**: Easy to trace calls through Lua
4. **Flexibility**: Natural Lua namespacing and dynamic calls
5. **Clean APIs**: plugin.lua files serve as clear public interfaces

## Example Implementation

See the updated `test_plugin` and new `math_consumer` plugin for complete examples of:
- Basic plugin with Lua namespace binding
- Dependent plugin using inter-plugin communication
- Lua headers serving as public API contracts

## Dependency System

The existing dependency resolution system remains unchanged:
- Plugins declare dependencies in `metadata.json`
- Load order is resolved automatically
- Dependent plugins load after their dependencies

## Migration Guide

To update existing plugins:
1. Change `pluginLoad`/`pluginShutdown` to return `bool`
2. Remove `ExportedPluginFunction` vector returns
3. Use `ctx.bind_function_namespace()` for cleaner organization
4. Create `plugin.lua` headers for public APIs
5. Use `ctx.call_lua<R>()` for inter-plugin calls