// MapperScriptPluginAPI.h
// Minimal API header for MapperScript plugins
//
// This header is designed to be included by plugin projects instead of the full PluginManager.h or ScriptManager.h.
// It avoids any dependency on sol2 or Lua headers, preventing symbol lookup errors and ensuring plugins only interact
// with MapperScript through a clean, stable interface.
//
// Why does this exist?
// --------------------
// Including sol2 or Lua headers in plugins causes the plugin to reference Lua C API symbols, which may not be exported
// by the main application (especially if Lua is linked statically). This leads to runtime errors like:
//   undefined symbol: lua_pushnil
//
// This header exposes only what plugins need: the ability to register (bind) C++ functions to the scripting environment.
//
// Usage:
//   1. Include this header in your plugin source files.
//   2. Use the provided MapperScriptPluginContext to register your functions.
//   3. Do NOT include sol2, Lua, or PluginManager.h in your plugin.
//

// Example plugin usage:
//   extern "C" int pluginInit(MapperScriptPluginContext* ctx) {
//       // Register a global function
//       ctx->bind_function("cpp_add_two_numbers", (void*)&cpp_add_two_numbers);
//       // Register a function in a namespace
//       ctx->bind_function_namespace("math", "add", (void*)&cpp_add_two_numbers);
//       return 0;
//   }

#pragma once
#include <string>

// Minimal plugin API for MapperScript plugins
// This struct is passed from the host to the plugin at load time.
// All function registration must go through these function pointers for ABI safety.

struct MapperScriptPluginContext {
    // Register a function globally
    void bind_function(const char* name, void* func_ptr);

    // Register a function in a namespace
    void bind_function_namespace(const char* ns, const char* name, void* func_ptr);
};


// See above for example usage in a plugin.