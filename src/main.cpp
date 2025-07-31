// Copyright (c) 2025 Lachlan McKenna
// All rights reserved. No part of this code may be used, copied, or distributed without permission.
#include <sol/sol.hpp>
#include <iostream>

int main() {


    sol::state lua; // Create Lua state
    lua.open_libraries(sol::lib::base); // Load basic Lua libraries

    // Run a simple Lua snippet
    lua.script(R"(
        print("Hello from Lua!")
        x = 42
    )");

    // Read variable from Lua
    const int x = lua["x"];
    std::cout << "Value of x from Lua: " << x << "\n";

    // Bind a C++ function to Lua
    lua.set_function("cpp_hello", []() {
        std::cout << "Hello from C++!\n";
    });

    // Call C++ function from Lua
    lua.script("cpp_hello()");
}
