# Coding Practices

## Code Style and Conventions

### Naming Conventions

#### Classes and Structs
- **PascalCase** for class names: `ScriptManager`, `PluginManager`
- **camelCase** for struct members: `pluginContext`, `ExportedPluginFunction`

#### Functions and Methods
- **snake_case** for function names: `load_script()`, `bind_function()`
- **camelCase** for some methods: `loadPluginsFromDir()`, `CheckIfPluginExists()`

#### Variables
- **snake_case** for local variables: `loaded_scripts_`, `file_watch_times_`
- **camelCase** for some member variables: `HotReload_StopRequested`, `Exec_running`
- **SCREAMING_SNAKE_CASE** for constants and enums: `PLUGIN_INIT_FAILURE`, `FILE_LOAD_SUCCESS`

#### Files and Directories
- **lowercase** with underscores: `plugin_loader.cpp`, `test_plugin.cpp`
- **PascalCase** for headers: `ScriptManager.h`, `PluginManager.h`

### Code Organization

#### Header Structure
```cpp
// Copyright notice
// All rights reserved statement

#pragma once

// System includes
#include <filesystem>
#include <string>

// Third-party includes
#include <sol/sol.hpp>
#include <nlohmann/json.hpp>

// Project includes
#include "Scripting/ScriptManager.h"

// Namespace aliases
namespace fs = std::filesystem;
using json = nlohmann::json;

// Class declaration
```

#### Implementation Structure
```cpp
// Copyright notice
// File path comment

// Project includes first
#include "PluginManager.h"

// System includes
#include <iostream>
#include <filesystem>

// Implementation
```

### Modern C++ Features

#### C++23 Standard
The project targets C++23 and makes use of modern features:

- `std::expected<T, E>` for error handling
- `std::format` for string formatting
- `std::filesystem` for path operations
- Range-based for loops
- Auto type deduction
- Smart pointers (`std::unique_ptr`, `std::shared_ptr`)

#### Error Handling Pattern
```cpp
// Preferred: std::expected for functions that may fail
std::expected<std::vector<plugin>, std::string> ResolveLoadOrder() const;

// Alternative: std::optional for nullable returns
std::optional<dynalo::library> lib;

// Exceptions for unexpected errors only
try {
    auto result = risky_operation();
} catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << '\n';
}
```

### Memory Management

#### RAII Principles
- Use smart pointers instead of raw pointers
- Automatic resource cleanup in destructors
- Exception-safe resource management

#### Examples
```cpp
// Good: Smart pointer for automatic cleanup
std::unique_ptr<pluginVector> loadedPlugins = std::make_unique<pluginVector>();

// Good: Optional for nullable handles
std::optional<dynalo::library> lib;

// Good: Move semantics for efficiency
loaded_scripts_.emplace(path, std::move(script));
```

### Template Usage

#### Function Templates
```cpp
// Template for generic function binding
template<typename Func>
void bind_function(const std::string& name, Func&& func) {
    lua_.set_function(name, std::forward<Func>(func));
}
```

#### Perfect Forwarding
- Use `std::forward<T>` for universal references
- Preserve value categories in template functions
- Avoid unnecessary copies

### Platform Abstraction

#### Preprocessor Macros
```cpp
#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif
```

#### Platform-Specific Code
- Minimize platform-specific code
- Use CMake for build-time platform detection
- Prefer standard library solutions when available

### Logging and Debugging

#### Structured Logging
```cpp
// Use utils class for consistent logging
utils::log(utils::LOG_SOURCE::PLUGIN_MANAGER, 
          "Plugin loaded successfully", 
          false);

// Color-coded output for different sources
utils::log(utils::LOG_SOURCE::SCRIPT_MANAGER, 
          "Script compilation failed", 
          true);  // Warning
```

#### Debug Output
```cpp
// Conditional debug output
#ifdef DEBUG
    std::cout << "[DEBUG] Variable value: " << value << '\n';
#endif

// Use std::format for complex formatting
utils::print("Loading plugin: {} v{}\n", name, version);
```

### Concurrency Patterns

#### Thread Safety
```cpp
// Atomic flags for thread communication
std::atomic_bool HotReload_StopRequested = false;

// Detached threads for background tasks
std::thread([this]() {
    // Background work
}).detach();

// Futures for async operations
std::future<void> run_script(const fs::path& path);
```

#### Resource Protection
- Use atomic operations for simple flags
- Avoid shared mutable state when possible
- Document thread safety guarantees

### Error Messages and User Feedback

#### Informative Error Messages
```cpp
std::cerr << "[PluginLoader] Plugin library not found: " << libPath << '\n';
std::cerr << "Failed to parse JSON: " << e.what() << '\n';
```

#### Consistent Prefixes
- `[PluginLoader]` for plugin-related messages
- `[ScriptManager]` for script-related messages
- Color coding through utils::log()

### Code Comments

#### Header Comments
```cpp
/// PluginManager
/// - Loads plugins from disk, reads manifest.
/// - Loads shared libraries (dynalo) per platform.
/// - Calls plugin init function: MapperPluginInit(PluginContext&).
```

#### Inline Comments
```cpp
// Check early if script is already loaded
if (loaded_scripts_.contains(path)) {
    return SMLoadResult::FILE_ALREADY_LOADED;
}

// TODO: Inter-plugin calls
// - Plugins query PluginManager for APIs of other plugins.
```

### Performance Considerations

#### Efficient Containers
```cpp
// Use unordered_map for O(1) lookups
std::unordered_map<std::filesystem::path, sol::load_result> loaded_scripts_;

// Use emplace for in-place construction
loaded_scripts_.emplace(path, std::move(script));
```

#### Move Semantics
```cpp
// Move expensive objects instead of copying
loadedPlugins->push_back(std::move(newPlugin));

// Use std::move in return statements for large objects
return std::move(PluginLoadOrder);
```

### Testing and Validation

#### Input Validation
```cpp
// Validate file existence before operations
if (!fs::exists(pluginDir) || !fs::is_directory(pluginDir)) {
    std::cerr << "Plugin directory does not exist: " << pluginDir << '\n';
    return false;
}
```

#### Defensive Programming
```cpp
// Check container validity before use
if (!loadedPlugins) {
    return false;
}

// Validate JSON structure
if (!metadata.is_object()) {
    std::cerr << "Invalid metadata format\n";
    return false;
}
```

## Development Workflow

### Build Configuration
- Use CMake for cross-platform builds
- Separate debug and release configurations
- Platform-specific library linking

### Code Organization
- Header-only templates in headers
- Implementation in corresponding .cpp files
- Minimal dependencies between modules

### Version Control
- Copyright notices in all files
- Consistent file headers
- Clear commit messages describing changes