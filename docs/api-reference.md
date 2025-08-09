# API Reference

## ScriptManager Class

### Overview
Manages Lua script loading, execution, and hot-reloading functionality.

### Public Methods

#### `SMInitResult init()`
Initializes the Lua state and loads basic libraries.

**Returns**: `SMInitResult::SUCCESS` or `SMInitResult::FAILURE`

#### `SMLoadResult load_script(const std::filesystem::path& path)`
Loads a Lua script from disk and prepares it for execution.

**Parameters**:
- `path`: Filesystem path to the Lua script

**Returns**:
- `FILE_LOAD_SUCCESS`: Script loaded successfully
- `FILE_LOAD_ERROR`: Error loading script
- `FILE_ALREADY_LOADED`: Script already in memory
- `TS_PMO`: Unknown error occurred

#### `std::future<void> run_script(const std::filesystem::path& path)`
Executes a loaded script asynchronously.

**Parameters**:
- `path`: Path to the previously loaded script

**Returns**: Future object for async execution

#### `void start_watcher_thread()`
Starts the file watcher thread for hot-reloading.

#### `void stop_watcher_thread()`
Stops the file watcher thread.

#### `bool save_loaded_scripts(const std::filesystem::path& json_out_path = "scripts.json")`
Saves currently loaded script paths to JSON file.

#### `bool restore_scripts_from_json(const std::filesystem::path& json_in_path = "scripts.json")`
Restores previously saved scripts from JSON file.

#### `template<typename Func> void bind_function(const std::string& name, Func&& func)`
Binds a C++ function to the global Lua namespace.

#### `template<typename Func> void bind_function_namespace(const std::string& ns, const std::string& name, Func&& func)`
Binds a C++ function to a specific Lua namespace.

---

## PluginManager Class

### Overview
Manages plugin discovery, loading, dependency resolution, and lifecycle.

### Public Types

#### `struct ExportedPluginFunction`
```cpp
struct ExportedPluginFunction {
    std::string name;
    std::any function;
};
```

#### `struct pluginContext`
Provides controlled access to ScriptManager for plugins.

**Methods**:
- `template<typename Func> void bind_function(const std::string& name, Func&& func)`
- `template<typename Func> void bind_function_namespace(const std::string& ns, const std::string& name, Func&& func)`

#### `enum class PLUGIN_INIT_FAILURE`
- `FAILURE`: Generic plugin initialization failure

#### `struct plugin`
Contains plugin metadata and runtime information.

**Members**:
- `bool loaded`: Plugin load status
- `std::string name`: Plugin name
- `std::string description`: Plugin description
- `std::string version`: Plugin version
- `json dependencies`: Plugin dependencies
- `fs::path folder_path`: Plugin directory path
- `fs::path lib_path`: Shared library path
- `fs::path luaScript_path`: Lua script path
- `std::optional<dynalo::library> lib`: Loaded library handle
- `RequiredPluginAPI RequiredAPI`: Required plugin functions
- `std::vector<ExportedPluginFunction> exportedFunctions`: Exported functions

### Public Methods

#### `void loadPluginsFromDir(const fs::path& plugin_dir) const`
Discovers and loads all plugins from the specified directory.

#### `bool CheckIfPluginExists(const std::string& name) const`
Checks if a plugin with the given name is already loaded.

#### `bool loadPluginMetadata(const std::filesystem::path& pluginDir) const`
Loads plugin metadata from a directory.

#### `static bool loadPluginLibrary(const plugin& newPlugin, ScriptManager& sm)`
Loads the shared library for a plugin.

#### `std::expected<std::reference_wrapper<plugin>, bool> GetPluginByName(const std::string& name) const`
Retrieves a plugin by name.

#### `std::expected<std::vector<std::reference_wrapper<plugin>>, std::string> ResolveLoadOrder() const`
Resolves plugin dependencies and determines load order.

---

## Plugin Interface

### Required Functions

Every plugin must export these C functions:

#### `pluginLoad(PluginManager::pluginContext& ctx)`
```cpp
PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                            PluginManager::PLUGIN_INIT_FAILURE> 
pluginLoad(PluginManager::pluginContext& ctx);
```

Called when the plugin is loaded. Use this to bind functions to Lua.

#### `pluginShutdown(PluginManager::pluginContext& ctx)`
```cpp
PLUGIN_EXPORT std::expected<std::vector<PluginManager::ExportedPluginFunction>, 
                            PluginManager::PLUGIN_INIT_FAILURE> 
pluginShutdown(PluginManager::pluginContext& ctx);
```

Called when the plugin is unloaded. Use this to clean up resources.

---

## Utility Classes

### utils Class

#### `template<typename... Args> static void print(std::format_string<Args...> fmt, Args&&... args)`
Formatted printing utility using C++20 format.

#### `enum class LOG_TYPE`
- `IS_WARNING = true`
- `IS_INFO = false`

#### `enum class LOG_SOURCE`
- `PLUGIN_MANAGER`
- `SCRIPT_MANAGER`
- `MAIN`
- `UI_MANAGER`
- `UNKNOWN`

#### `static void log(const LOG_SOURCE source, const std::string& message, const bool isWarning = false)`
Colored logging with source identification.

---

## Error Handling

### Return Types

The project uses modern C++ error handling patterns:

- `std::expected<T, E>`: For functions that may fail
- `std::optional<T>`: For functions that may not return a value
- `std::future<T>`: For asynchronous operations

### Exception Safety

- All public APIs provide basic exception safety
- Plugin functions should handle exceptions internally
- File operations use RAII for resource management

---

## Thread Safety

### ScriptManager
- Script execution is single-threaded (one script at a time)
- File watcher runs on separate thread
- Hot-reload operations are thread-safe

### PluginManager
- Plugin loading is not thread-safe
- Should be called from main thread only
- Plugin functions may be called from multiple threads

---

## Platform Support

### Supported Platforms
- Windows (MSVC)
- Linux (GCC/Clang)
- macOS (Clang)

### Platform-Specific Code
- Dynamic library loading handled by dynalo
- File path operations use std::filesystem
- Platform detection via CMake preprocessor definitions