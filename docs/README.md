# MapperScript

## Project Overview

MapperScript is a plugin-based scripting environment designed for the FLL2025 CaveBot robotics project. It provides a development platform for creating and testing Lua algorithms with hot-reloading capabilities and a modular plugin architecture.

## Key Features

- **Hot-Reloading Lua Environment**: Scripts are automatically reloaded when modified
- **Plugin-Based Architecture**: Extensible system with C++ plugins exposing functions to Lua
- **Cross-Platform Support**: Windows, Linux, and macOS compatibility
- **Dependency Management**: Plugin dependency resolution and load ordering
- **Script Management**: Load, execute, and manage multiple Lua scripts

## Architecture

The project follows a modular design with three main components:

1. **ScriptManager**: Handles Lua script loading, execution, and hot-reloading
2. **PluginManager**: Manages plugin discovery, loading, and dependency resolution
3. **Plugin System**: C++ shared libraries that expose functionality to Lua scripts

## Quick Start

1. Build the project using CMake
2. Place Lua scripts in the `scripts/` directory
3. Create plugins in the `plugins/` directory
4. Run MapperScript to start the scripting environment

## Documentation Structure

- [Project Structure](project-structure.md) - Detailed breakdown of directories and files
- [Plugin Development](plugin-development.md) - Guide for creating plugins
- [API Reference](api-reference.md) - Class and function documentation
- [Coding Practices](coding-practices.md) - Development conventions used in the project

## Dependencies

- **Lua 5.4**: Scripting engine
- **Sol2**: C++/Lua binding library
- **nlohmann/json**: JSON parsing for configuration
- **dynalo**: Cross-platform dynamic library loading

## License

Copyright (c) 2025 Lachlan McKenna. All rights reserved.