# Project Structure

## Directory Layout

```
MapperScript/
├── cmake-build-debug/          # Debug build artifacts
├── cmake-build-release/        # Release build artifacts
├── include/                    # Header files
│   ├── plugins/
│   │   └── PluginManager.h     # Plugin system management
│   ├── Scripting/
│   │   └── ScriptManager.h     # Lua script management
│   ├── utils/
│   │   └── utils.hpp           # Utility functions and logging
│   └── MapperEngine.h          # Main engine header
├── plugins/                    # Plugin directory
│   └── test_plugin/            # Example plugin
│       ├── src/
│       │   └── test_plugin.cpp # Plugin implementation
│       ├── metadata.json       # Plugin metadata
│       ├── plugin.lua          # Plugin Lua interface
│       └── CMakeLists.txt      # Plugin build configuration
├── scripts/                    # Lua scripts directory
│   └── script.lua              # Example script
├── src/                        # Source files
│   ├── plugins/
│   │   └── pluginLoader.cpp    # Plugin loading implementation
│   ├── Scripting/
│   │   └── ScriptManager.cpp   # Script management implementation
│   └── main.cpp                # Application entry point
├── vendored/                   # Third-party dependencies
│   ├── dynalo/                 # Dynamic library loading
│   ├── lua/                    # Lua 5.4 (Windows)
│   ├── linux_lua/              # Lua 5.4 (Linux)
│   ├── osx_lua/                # Lua 5.4 (macOS)
│   └── sol2/                   # Sol2 Lua binding library
└── CMakeLists.txt              # Root build configuration
```

## Core Components

### ScriptManager (`include/Scripting/ScriptManager.h`)
- Manages Lua state and script lifecycle
- Provides hot-reloading functionality
- Handles script execution and error management
- Exposes function binding interface for plugins

### PluginManager (`include/plugins/PluginManager.h`)
- Discovers and loads plugins from directories
- Resolves plugin dependencies and load order
- Manages plugin lifecycle (load/unload)
- Provides plugin context for Lua binding

### Plugin System
- Shared libraries (.so/.dll/.dylib) with standardized interface
- Each plugin exports `pluginLoad` and `pluginShutdown` functions
- Plugins can bind C++ functions to Lua through the plugin context
- Metadata-driven configuration with dependency support

## Build System

The project uses CMake with platform-specific configurations:

- **Root CMakeLists.txt**: Main application build
- **Plugin CMakeLists.txt**: Individual plugin builds
- Platform detection for library linking
- Automatic resource copying (scripts, plugin files)

## Data Flow

1. **Initialization**: ScriptManager initializes Lua state
2. **Plugin Discovery**: PluginManager scans plugin directories
3. **Dependency Resolution**: Plugins loaded in dependency order
4. **Function Binding**: Plugins register C++ functions with Lua
5. **Script Execution**: Lua scripts can call plugin functions
6. **Hot Reload**: File watcher detects changes and reloads scripts

## File Naming Conventions

- **Headers**: `.h` extension in `include/` directory
- **Sources**: `.cpp` extension in `src/` directory
- **Plugins**: `plugin.so/dll/dylib` with `metadata.json` and `plugin.lua`
- **Scripts**: `.lua` extension in `scripts/` directory