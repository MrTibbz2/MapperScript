# Plugin Development Guide

## Plugin Architecture

Plugins in MapperScript are shared libraries that extend the Lua scripting environment with C++ functionality. Each plugin consists of:

1. **C++ Implementation**: Shared library with standardized interface
2. **Metadata File**: JSON configuration with plugin information
3. **Lua Interface**: Optional Lua module for higher-level functionality

## Creating a Plugin

### 1. Directory Structure

```
plugins/your_plugin/
├── src/
│   └── your_plugin.cpp     # Plugin implementation
├── metadata.json           # Plugin configuration
├── plugin.lua              # Lua interface (optional)
└── CMakeLists.txt          # Build configuration
```

### 2. Plugin Implementation

```cpp
#include "plugins/PluginManager.h"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

// Your plugin functions
int your_function(int a, int b) {
    return a + b;
}

extern "C" {
    // Required: Plugin load function
    PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                                PluginManager::PLUGIN_INIT_FAILURE> 
    pluginLoad(PluginManager::pluginContext& ctx) {
        // Bind your functions to Lua
        ctx.bind_function("your_function", &your_function);
        
        // Return empty vector on success
        std::vector<PluginManager::ExportedPluginFunction> ret;
        return ret;
    }

    // Required: Plugin shutdown function
    PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                                PluginManager::PLUGIN_INIT_FAILURE> 
    pluginShutdown(PluginManager::pluginContext& ctx) {
        // Cleanup resources here
        std::vector<PluginManager::ExportedPluginFunction> ret;
        return ret;
    }
}
```

### 3. Metadata Configuration

```json
{
  "name": "your_plugin",
  "version": "1.0.0",
  "enabled": true,
  "author": "Your Name",
  "description": "Description of your plugin",
  "dependencies": [
    {
      "name": "dependency_plugin",
      "version": ">=1.0.0"
    }
  ]
}
```

### 4. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(plugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES src/your_plugin.cpp)

add_library(plugin SHARED ${SOURCES})

target_include_directories(plugin PRIVATE
    ../../include
    ../../vendored/lua/include
    ../../vendored/linux_lua/include
    ../../vendored/osx_lua/include
    ../../vendored/sol2/include
)

# Platform-specific linking (copy from test_plugin)
# ... platform detection and Lua linking ...

set_target_properties(plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/your_plugin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/your_plugin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/your_plugin"
    POSITION_INDEPENDENT_CODE ON
    PREFIX ""
)
```

## Plugin Context API

The `pluginContext` provides access to the Lua environment:

### Function Binding

```cpp
// Bind function to global namespace
ctx.bind_function("function_name", &your_function);

// Bind function to namespace
ctx.bind_function_namespace("namespace", "function_name", &your_function);
```

### Supported Function Types

- Free functions
- Lambda expressions
- Function objects
- Member functions (with object instance)

## Dependency Management

### Declaring Dependencies

```json
{
  "dependencies": [
    {
      "name": "math_plugin",
      "version": ">=2.0.0"
    },
    {
      "name": "io_plugin",
      "version": "1.5.0"
    }
  ]
}
```

### Load Order

The PluginManager automatically resolves dependencies and loads plugins in the correct order. Plugins with no dependencies load first, followed by dependent plugins.

## Best Practices

### Error Handling

- Use `std::expected` return types for plugin functions
- Handle exceptions gracefully in plugin code
- Validate input parameters from Lua

### Memory Management

- Avoid raw pointers in plugin interfaces
- Use RAII for resource management
- Clean up resources in `pluginShutdown`

### Naming Conventions

- Use descriptive function names
- Prefix plugin-specific functions to avoid conflicts
- Follow C++ naming conventions for internal code

### Performance

- Minimize work in `pluginLoad` and `pluginShutdown`
- Cache expensive computations
- Consider thread safety for concurrent access

## Example: Math Plugin

```cpp
// math_plugin.cpp
#include "plugins/PluginManager.h"
#include <cmath>

double calculate_distance(double x1, double y1, double x2, double y2) {
    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

extern "C" {
    PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                                PluginManager::PLUGIN_INIT_FAILURE> 
    pluginLoad(PluginManager::pluginContext& ctx) {
        ctx.bind_function_namespace("math", "distance", &calculate_distance);
        ctx.bind_function_namespace("math", "sqrt", static_cast<double(*)(double)>(&std::sqrt));
        
        std::vector<PluginManager::ExportedPluginFunction> ret;
        return ret;
    }

    PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                                PluginManager::PLUGIN_INIT_FAILURE> 
    pluginShutdown(PluginManager::pluginContext& ctx) {
        std::vector<PluginManager::ExportedPluginFunction> ret;
        return ret;
    }
}
```

Usage in Lua:
```lua
local distance = math.distance(0, 0, 3, 4)  -- Returns 5.0
local root = math.sqrt(16)                   -- Returns 4.0
```