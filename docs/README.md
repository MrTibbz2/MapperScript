# MapperScript

## Project Overview

MapperScript is a plugin-based scripting environment designed origionally by lachlan mckenna, and used for the FLL2025 CaveBot robotics project. It provides a development platform for creating and testing Lua algorithms with hot-reloading capabilities and a modular plugin architecture.

## Key Features
- **remote backend**: 
- **Plugin-Based Architecture**: Extensible system with C++ plugins exposing functions to Lua
- **Cross-Platform Support**: Windows, Linux, and macOS compatibility (currently working on windows compatibility.)
- **Dependency Management**: Plugin dependency resolution and load ordering
- **Script Management**: Load, execute, and manage multiple Lua scripts

## Architecture

The project follows a modular design with four main components:

1. **ScriptManager**: Handles Lua script loading, execution, and hot-reloading
2. **PluginManager**: Manages plugin discovery, loading, and dependency resolution
3. **Plugin System**: C++ shared libraries that expose functionality to Lua scripts
4. **web interface**: allows for a frontend connection over web api.

## Quick Start

1. Build the project using CMake
2. Place Lua scripts in the `scripts/` directory
3. Create plugins in the `plugins/` directory
4. Run MapperScript to start the scripting environment


## Dependencies

- **Lua 5.4**: Scripting engine
- **Sol2**: C++/Lua binding library
- **libjson-rpc-cxx and crow**: for client connection.
- **nlohmann/json**: JSON parsing for configuration
- **dynalo**: Cross-platform dynamic library loading

## License

Copyright (c) 2025 Lachlan McKenna. All rights reserved.
